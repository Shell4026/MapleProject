#include "UserManager.h"
#include "UserManager.h"
#include "DBThread.h"
#include "Packet/PlayerJoinPacket.hpp"
#include "Packet/PlayerLeavePacket.hpp"

#include "Core/ThreadPool.h"
#include "Core/Logger.h"

#include <string>
namespace sh::game
{
	UserManager::UserManager()
	{
		auto dbThread = DBThread::GetInstance();
		if (dbThread->Init("users.db"))
		{
			queryUserSQL = dbThread->CreateSQL("SELECT id, nickname, lastWorld, lastX, lastY, lastIP, lastPort FROM Users WHERE nickname = ?;");
			insertUserSQL = dbThread->CreateSQL("INSERT INTO Users (nickname, lastIp, lastPort) VALUES (?, ?, ?);");
			updateUserSQL = dbThread->CreateSQL("UPDATE Users SET lastIp = ?, lastPort = ? WHERE id = ?;");
			queryUserInventorySQL = dbThread->CreateSQL("SELECT ownerId, slotIdx, itemId, count, instanceId FROM UserInventory WHERE ownerId = ?;");
			upsertItemSQL = dbThread->CreateSQL(
				R"(
					INSERT INTO UserInventory (ownerId, slotIdx, itemId, count)
					VALUES (?, ?, ?, ?)
					ON CONFLICT(ownerId, slotIdx)
					DO UPDATE SET itemId = excluded.itemId, count = excluded.count;)");
			deleteItemSQL = dbThread->CreateSQL("DELETE FROM UserInventory WHERE ownerId = ? AND slotIdx = ?;");
		}
	}
	UserManager::~UserManager()
	{
		for (auto& future : pendingJoins)
			future.get();
		while (!pendingInventory.empty())
		{
			pendingInventory.front().future.get();
			pendingInventory.pop();
		}
	}
	SH_USER_API auto UserManager::ProcessPlayerJoinUdp(const PlayerJoinPacket& packet, const core::UUID& token, const Endpoint& ep) -> bool
	{
		{
			auto it = userUUIDs.find(token);
			if (it != userUUIDs.end())
				return false;
		}

		auto it = userUdpEndpoints.find(ep);
		if (it == userUdpEndpoints.end())
		{
			const std::string nickname = packet.GetNickName();
			SH_INFO_FORMAT("A player({}) has joined - {}:{}", nickname, ep.ip, ep.port);

			User::CreateInfo userInfo{};
			userInfo.nickname = nickname;
			userInfo.ip = ep.ip;
			userInfo.port = ep.port;
			userInfo.uuid = token;

			auto& [pair, success] = pendingUsers.insert({ token, std::move(userInfo) });
			if (!success)
				return false;

			userUdpEndpoints.insert({ ep, token });
		}
		else
			return false;
		return true;
	}

	SH_USER_API auto UserManager::ProcessPlayerJoinTcp(std::unique_ptr<network::TcpSocket>&& tcpSocket, const core::UUID& token, const Endpoint& ep) -> bool
	{
		auto it = pendingUsers.find(token);
		if (it == pendingUsers.end())
			return false;

		auto& userInfo = it->second;
		userInfo.tcpSocket = std::move(tcpSocket);

		auto future = DBThread::GetInstance()->AddTask(
			[this, &db = DBThread::GetInstance()->GetDB(), ep, token, nickname = userInfo.nickname]() -> PendingJoin
			{
				int64_t userId = 0;
				PendingJoin pendingJoin{ core::UUID::GenerateEmptyUUID() };

				auto result = db.Query(queryUserSQL, nickname);
				if (result.empty())
				{
					if (!db.Execute(insertUserSQL, nickname, ep.ip, ep.port))
					{
						SH_ERROR_FORMAT("Failed to insert into users: ({})", nickname);
						return pendingJoin;
					}
					result = db.Query(queryUserSQL, nickname);
					if (result.empty())
						return pendingJoin;
					userId = std::get<0>(result[0][0]);
				}
				else
				{
					userId = std::get<0>(result[0][0]);
					if (!db.Execute(updateUserSQL, ep.ip, ep.port, userId))
					{
						SH_ERROR_FORMAT("Failed to update user: ({})", nickname);
						return pendingJoin;
					}
				}
				pendingJoin.id = userId;
				pendingJoin.userUUID = token;
				pendingJoin.inventory = LoadInventory(db, userId);

				return pendingJoin;
			}
		);
		pendingJoins.push_back(std::move(future));
		return true;
	}

	SH_USER_API auto UserManager::ProcessPlayerLeave(const PlayerLeavePacket& packet) -> bool
	{
		if (auto it = pendingUsers.find(packet.user); it != pendingUsers.end())
		{
			pendingUsers.erase(it);
			return true;
		}
		User* const userPtr = GetUser(packet.user);
		if (userPtr == nullptr)
			return false;

		return ProcessPlayerLeave(*userPtr);
	}

	SH_USER_API auto UserManager::ProcessPlayerLeave(User& user) -> bool
	{
		UpdateInventoryDB(user);

		UserEvent evt{};
		evt.user = &user;
		evt.type = UserEvent::Type::LeaveUser;
		bus.Publish(evt);

		EraseUser(user);
		return true;
	}

	SH_USER_API void UserManager::KickUser(User& user)
	{
		auto it = userUUIDs.find(user.GetUserUUID());
		if (it == userUUIDs.end())
			return;

		UserEvent evt{};
		evt.user = &user;
		evt.type = UserEvent::Type::KickUser;
		bus.Publish(evt);

		EraseUser(user);
	}

	SH_USER_API void UserManager::KickUser(const core::UUID& userUUID)
	{
		auto it = userUUIDs.find(userUUID);
		if (it == userUUIDs.end())
			return;

		User* user = users[it->second];

		UserEvent evt{};
		evt.user = user;
		evt.type = UserEvent::Type::KickUser;
		bus.Publish(evt);

		EraseUser(*user);
	}

	SH_USER_API void UserManager::Tick(float dt)
	{
		if (!pendingJoins.empty())
		{
			for (auto it = pendingJoins.begin(); it != pendingJoins.end();)
			{
				std::future<PendingJoin>& future = *it;
				if (future.wait_for(std::chrono::milliseconds(0)) != std::future_status::ready)
				{
					++it;
					continue;
				}
				PendingJoin pendingJoin = future.get();
				const core::UUID& token = pendingJoin.userUUID;
				if (token.IsEmpty()) // 실패
				{
					it = pendingJoins.erase(it);
					continue;
				}

				auto pendingUsersIt = pendingUsers.find(token);
				if (pendingUsersIt == pendingUsers.end())
				{
					it = pendingJoins.erase(it);
					continue;
				}

				User::CreateInfo& ci = pendingUsersIt->second;
				ci.id = pendingJoin.id;
				ci.uuid = token;
				ci.inventory = std::move(pendingJoin.inventory);

				User* user = userPool.Allocate();
				new (user) User(std::move(ci));

				UserIdx idx = users.size();
				users.push_back(user);

				userUUIDs.insert_or_assign(user->GetUserUUID(), idx);

				UserEvent evt{};
				evt.user = user;
				evt.type = UserEvent::Type::JoinUser;

				bus.Publish(evt);

				pendingUsers.erase(pendingUsersIt);
				it = pendingJoins.erase(it);
			}
		}
		while (!pendingInventory.empty())
		{
			if (pendingInventory.front().future.wait_for(std::chrono::milliseconds(0)) != std::future_status::ready)
				break;
			auto pending = std::move(pendingInventory.front());
			pendingInventory.pop();
			auto userPtr = GetUser(pending.userUUID);
			if (userPtr == nullptr)
				continue;
			Inventory& inven = userPtr->GetInventory();
			bool bSuccess = pending.future.get();
			if (bSuccess)
				inven.ClearDirtySlots();
		}

		std::queue<User*> deadUser;
		for (auto user : users)
		{
			user->Tick(dt);
			if (user->GetHeartbeat() == 0)
				deadUser.push(user);
		}
		while (!deadUser.empty())
		{
			User* user = deadUser.front();
			deadUser.pop();

			ProcessPlayerLeave(*user);
		}
	}

	SH_USER_API auto UserManager::GetUser(const core::UUID& uuid) const -> User*
	{
		auto it = userUUIDs.find(uuid);
		if (it == userUUIDs.end())
			return nullptr;

		UserIdx idx = it->second;

		return users[idx];
	}
	void sh::game::UserManager::EraseUser(User& user)
	{
		auto itUUID = userUUIDs.find(user.GetUserUUID());
		if (itUUID == userUUIDs.end())
			return;
		userUdpEndpoints.erase(Endpoint{ user.GetIp(), user.GetPort() });

		UserIdx idx = itUUID->second;
		UserIdx last = users.size() - 1;

		User* const userPtr = users[idx];

		userUUIDs.erase(itUUID);

		if (idx != last)
		{
			User* lastUser = users[last];
			users[idx] = lastUser;
			userUUIDs[lastUser->GetUserUUID()] = idx;
		}
		users.pop_back();

		userPtr->~User();
		userPool.DeAllocate(userPtr);
	}
	auto UserManager::LoadInventory(Database& db, uint64_t userId) const -> Inventory
	{
		auto result = db.Query(queryUserInventorySQL, userId);
		if (result.empty())
			return {};

		Inventory inventory{};
		for (auto& row : result)
		{
			int ownerId = std::get<0>(row[0]);
			int slotIdx = std::get<0>(row[1]);
			int itemId = std::get<0>(row[2]);
			int quantity = std::get<0>(row[3]);
			int64_t instanceId = std::get<0>(row[4]);
			inventory.SetItem(itemId, slotIdx, quantity);
		}
		return inventory;
	}
	void UserManager::UpdateInventoryDB(User& user)
	{
		const int64_t userId = user.GetId();
		Inventory& inventory = user.GetInventory();

		auto future = DBThread::GetInstance()->AddTask(
			[this, userId, slots = inventory.GetSlots(), dirtySlots = inventory.GetDirtySlots(), &db = DBThread::GetInstance()->GetDB()]() -> bool
			{
				if (!db.Execute("BEGIN TRANSACTION;"))
				{
					SH_ERROR("BEGIN TRANSACTION;");
					return false;
				}

				bool bSuccess = true;
				for (int slotIdx : dirtySlots)
				{
					const auto& slot = slots[slotIdx];
					if (slot.quantity <= 0)
						bSuccess = db.Execute(deleteItemSQL, userId, slotIdx);
					else
						bSuccess = db.Execute(upsertItemSQL, userId, slotIdx, slot.itemId, slot.quantity);

					if (!bSuccess)
						break;
				}
				if (bSuccess)
					bSuccess = db.Execute("Commit;");
				if (!bSuccess)
				{
					SH_ERROR_FORMAT("Failed to update inventory: user({})", userId);
					db.Execute("ROLLBACK;");
					return false;
				}
				return true;
			}
		);
		PendingInventory pending{};
		pending.userUUID = user.GetUserUUID();
		pending.future = std::move(future);
		pendingInventory.push(std::move(pending));
	}
}//namespace