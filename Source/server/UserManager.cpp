#include "UserManager.h"
#include "Packet/PlayerJoinPacket.hpp"
#include "DBThread.h"

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
		for (auto& [ep, pending] : pendingJoin)
			pending.future.get();
		while (!pendingInventory.empty())
		{
			pendingInventory.front().future.get();
			pendingInventory.pop();
		}
	}
	SH_USER_API auto UserManager::ProcessPlayerJoin(const PlayerJoinPacket& packet, const Endpoint& ep) -> bool
	{
		if (pendingJoin.find(ep) != pendingJoin.end())
			return false;

		auto it = userEndpoints.find(ep);
		if (it == userEndpoints.end())
		{
			SH_INFO_FORMAT("A player({}) has joined - {}:{}", packet.GetNickName(), ep.ip, ep.port);

			auto future = DBThread::GetInstance()->AddTask(
				[this, nickname = packet.GetNickName(), ep = ep, &db = DBThread::GetInstance()->GetDB()]() -> User*
				{
					auto result = db.Query(queryUserSQL, nickname);
					int64_t userId = 0;
					if (result.empty())
					{
						if (!db.Execute(insertUserSQL, nickname, ep.ip, ep.port))
						{
							SH_ERROR_FORMAT("Failed to insert into users: ({})", nickname);
							return nullptr;
						}
						result = db.Query(queryUserSQL, nickname);
						if (result.empty())
							return nullptr;
						userId = std::get<0>(result[0][0]);
					}
					else
					{
						userId = std::get<0>(result[0][0]);
						if (!db.Execute(updateUserSQL, ep.ip, ep.port, userId))
						{
							SH_ERROR_FORMAT("Failed to update user: ({})", nickname);
							return nullptr;
						}
					}
					std::unique_lock<std::mutex> lock{ poolMutex };
					User* user = userPool.Allocate();
					lock.unlock();

					new (user) User{ userId, ep.ip, ep.port };
					user->SetNickname(std::move(nickname));
					user->SetInventory(LoadInventory(db, userId));

					return user;
				}
			);
			pendingJoin[ep] = PendingJoin{ std::move(future), false };
		}
		else
			return false;
		return true;
	}

	SH_USER_API auto UserManager::ProcessPlayerLeave(const PlayerLeavePacket& packet, const Endpoint& ep) -> bool
	{
		if (auto it = pendingJoin.find(ep); it != pendingJoin.end())
		{
			it->second.bLeave = true;
			return true;
		}

		auto it = userEndpoints.find(ep);
		if (it == userEndpoints.end())
			return false;

		User* const userPtr = users[it->second];
		UpdateInventoryDB(*userPtr);

		UserEvent evt{};
		evt.user = userPtr;
		evt.type = UserEvent::Type::LeaveUser;
		bus.Publish(evt);

		EraseUser(*userPtr);
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

	SH_USER_API void UserManager::KickUser(const Endpoint& ep)
	{
		auto it = userEndpoints.find(ep);
		if (it == userEndpoints.end())
			return;

		User* user = users[it->second];

		UserEvent evt{};
		evt.user = user;
		evt.type = UserEvent::Type::KickUser;
		bus.Publish(evt);

		EraseUser(*user);
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

	SH_USER_API void UserManager::Update()
	{
		if (!pendingJoin.empty())
		{
			for (auto it = pendingJoin.begin(); it != pendingJoin.end();)
			{
				const Endpoint& ep = it->first;
				std::future<User*>& future = it->second.future;
				if (future.wait_for(std::chrono::milliseconds(0)) != std::future_status::ready)
				{
					++it;
					continue;
				}
				User* user = future.get();
				if (user == nullptr)
				{
					it = pendingJoin.erase(it);
					continue;
				}

				if (it->second.bLeave) // 들어오던 중 나감
				{
					user->~User();
					std::lock_guard<std::mutex> lock{ poolMutex };
					userPool.DeAllocate(user);
				}
				else
				{
					UserIdx idx = users.size();
					users.push_back(user);

					userUUIDs.insert_or_assign(user->GetUserUUID(), idx);
					userEndpoints.insert_or_assign(ep, idx);

					UserEvent evt{};
					evt.user = user;
					evt.type = UserEvent::Type::JoinUser;

					bus.Publish(evt);
				}
				it = pendingJoin.erase(it);
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
	}

	SH_USER_API auto UserManager::GetUser(const Endpoint& ep) const -> User*
	{
		auto it = userEndpoints.find(ep);
		if (it == userEndpoints.end())
			return nullptr;

		UserIdx idx = it->second;

		return users[idx];
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
		auto itEp = userEndpoints.find(Endpoint{ user.GetIp(), user.GetPort() });
		if (itEp == userEndpoints.end())
			return;
		if (itUUID->second != itEp->second)
			return;

		UserIdx idx = itUUID->second;
		UserIdx last = users.size() - 1;

		User* const userPtr = users[idx];

		userUUIDs.erase(itUUID);
		userEndpoints.erase(itEp);

		if (idx != last)
		{
			User* lastUser = users[last];
			users[idx] = lastUser;
			userUUIDs[lastUser->GetUserUUID()] = idx;
			userEndpoints[Endpoint{ lastUser->GetIp(), lastUser->GetPort() }] = idx;
		}
		users.pop_back();

		userPtr->~User();
		std::lock_guard<std::mutex> lock{ poolMutex };
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