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
#include <queue>
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

		SH_USER_API auto GetUser(const Endpoint& ep) const -> User*;
		SH_USER_API auto GetUser(const core::UUID& uuid) const -> User*;
		SH_USER_API auto GetUsers() const -> const std::vector<User*>& { return users; }
	private:
		void EraseUser(User& user);
		auto LoadInventory(Database& db, uint64_t userId) const -> Inventory;
		void UpdateInventoryDB(User& user);
	public:
		core::EventBus bus;
	private:
		Database::SQL queryUserSQL;
		Database::SQL insertUserSQL;
		Database::SQL updateUserSQL;
		Database::SQL queryUserInventorySQL;
		Database::SQL upsertItemSQL;
		Database::SQL deleteItemSQL;

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
		struct PendingInventory
		{
			core::UUID userUUID = core::UUID::GenerateEmptyUUID();
			std::future<bool> future;
		};
		std::queue<PendingInventory> pendingInventory;
		std::mutex poolMutex;
	};
}//namespace