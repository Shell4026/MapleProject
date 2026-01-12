#include "UserManager.h"
#include "UserManager.h"
#include "Packet/PlayerJoinPacket.hpp"

#include "Core/ThreadPool.h"
#include "Core/Logger.h"

#include <string>
namespace sh::game
{
	UserManager::UserManager() :
		db("users.db")
	{
		if (db.IsOpen())
		{
			queryUserSQL = db.CreateSQL("SELECT * FROM Users WHERE nickname = ?;");
			insertUserSQL = db.CreateSQL("INSERT INTO Users (nickname, lastIp, lastPort) VALUES (?, ?, ?);");
			updateUserSQL = db.CreateSQL("UPDATE Users SET lastIp = ?, lastPort = ? WHERE id = ?;");
		}
	}
	UserManager::~UserManager()
	{
		for (auto& [ep, pending] : pendingJoin)
			pending.future.get();
	}
	SH_USER_API auto UserManager::ProcessPlayerJoin(const PlayerJoinPacket& packet, const Endpoint& ep) -> bool
	{
		if (pendingJoin.find(ep) != pendingJoin.end())
			return false;

		auto it = userEndpoints.find(ep);
		if (it == userEndpoints.end())
		{
			SH_INFO_FORMAT("A player({}) has joined - {}:{}", packet.GetNickName(), ep.ip, ep.port);

			auto future = core::ThreadPool::GetInstance()->AddContinousTask(
				[this, nickname = packet.GetNickName(), ep = ep]() -> User*
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
		if (pendingJoin.empty())
			return;
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

	SH_USER_API auto UserManager::GetUser(const Endpoint& ep) -> User*
	{
		auto it = userEndpoints.find(ep);
		if (it == userEndpoints.end())
			return nullptr;

		UserIdx idx = it->second;

		return users[idx];
	}
	SH_USER_API auto UserManager::GetUser(const core::UUID& uuid) -> User*
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
}//namespace