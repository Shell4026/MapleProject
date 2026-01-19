#pragma once
#include "Export.h"
#include "User.h"
#include "EndPoint.hpp"
#include "Database.h"
#include "UserEvent.hpp"

#include "Core/UUID.h"
#include "Core/Memory/MemoryPool.hpp"
#include "Core/EventBus.h"

#include "Network/TcpSocket.h"

#include <unordered_map>
#include <list>
#include <future>
#include <cstdint>
#include <queue>
#include <memory>
namespace sh::game
{
	class PlayerJoinPacket;
	class PlayerLeavePacket;
	class UserManager
	{
	public:
		SH_USER_API UserManager();
		SH_USER_API ~UserManager();

		SH_USER_API auto ProcessPlayerJoinUdp(const PlayerJoinPacket& packet, const core::UUID& token, const Endpoint& ep) -> bool;
		SH_USER_API auto ProcessPlayerJoinTcp(std::unique_ptr<network::TcpSocket>&& tcpSocket, const core::UUID& token, const Endpoint& ep) -> bool;
		SH_USER_API auto ProcessPlayerLeave(const PlayerLeavePacket& packet) -> bool;
		SH_USER_API auto ProcessPlayerLeave(User& user) -> bool;
		SH_USER_API void KickUser(User& user);
		SH_USER_API void KickUser(const core::UUID& userUUID);

		SH_USER_API void Tick(float dt);

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
		std::unordered_map<Endpoint, core::UUID> userUdpEndpoints;

		std::unordered_map<core::UUID, User::CreateInfo> pendingUsers;
		struct PendingJoin
		{
			core::UUID userUUID;
			int64_t id;
			Inventory inventory;
		};
		std::list<std::future<PendingJoin>> pendingJoins;
		struct PendingInventory
		{
			core::UUID userUUID = core::UUID::GenerateEmptyUUID();
			std::future<bool> future;
		};
		std::queue<PendingInventory> pendingInventory;
	};
}//namespace