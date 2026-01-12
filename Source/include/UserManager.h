#pragma once
#include "Export.h"
#include "User.h"
#include "EndPoint.hpp"
#include "Database.h"
#include "UserEvent.hpp"

#include "Core/UUID.h"
#include "Core/Memory/MemoryPool.hpp"
#include "Core/EventBus.h"

#include <unordered_map>
#include <list>
#include <future>
#include <cstdint>
#include <mutex>
namespace sh::game
{
	class PlayerJoinPacket;
	class PlayerLeavePacket;
	class UserManager
	{
	public:
		SH_USER_API UserManager();
		SH_USER_API ~UserManager();

		SH_USER_API auto ProcessPlayerJoin(const PlayerJoinPacket& packet, const Endpoint& ep) -> bool;
		SH_USER_API auto ProcessPlayerLeave(const PlayerLeavePacket& packet, const Endpoint& ep) -> bool;
		SH_USER_API void KickUser(User& user);
		SH_USER_API void KickUser(const Endpoint& ep);
		SH_USER_API void KickUser(const core::UUID& userUUID);

		SH_USER_API void Update();

		SH_USER_API auto GetUser(const Endpoint& ep) -> User*;
		SH_USER_API auto GetUser(const core::UUID& uuid) -> User*;
		SH_USER_API auto GetUsers() const -> const std::vector<User*>& { return users; }
		SH_USER_API auto GetDB() -> Database& { return db; }
	private:
		void EraseUser(User& user);
	public:
		core::EventBus bus;
	private:
		Database db;
		Database::SQL queryUserSQL;
		Database::SQL insertUserSQL;
		Database::SQL updateUserSQL;

		core::memory::MemoryPool<User, 32> userPool;
		std::vector<User*> users;
		using UserIdx = uint64_t;
		std::unordered_map<core::UUID, UserIdx> userUUIDs;
		std::unordered_map<Endpoint, UserIdx> userEndpoints;

		struct PendingJoin
		{
			std::future<User*> future;
			bool bLeave = false;
		};
		std::unordered_map<Endpoint, PendingJoin> pendingJoin;
		
		std::mutex poolMutex;
	};
}//namespace