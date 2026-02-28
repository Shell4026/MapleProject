#pragma once
#include "Export.h"
#include "Inventory.h"

#include "Core/UUID.h"
#include "Network/TcpSocket.h"

#include <string>
#include <cstdint>
namespace sh::game
{
	class User
	{
	public:
		struct CreateInfo
		{
			int64_t id;
			std::string nickname;
			std::string ip;
			uint16_t port;
			core::UUID uuid = core::UUID::Generate();
			Inventory inventory;
			std::unique_ptr<network::TcpSocket> tcpSocket;
		};
	public:
		SH_USER_API User(CreateInfo&& ci);
		SH_USER_API User(User&& other) noexcept;

		SH_USER_API auto operator=(User&& other) noexcept -> User&;

		SH_USER_API void SetNickname(const std::string& name);
		SH_USER_API void SetNickname(std::string&& name) noexcept;
		SH_USER_API void SetCurrentWorldUUID(const core::UUID& worldUUID);
		SH_USER_API void SetPendingSpawnPortalId(int portalId) { pendingSpawnPortalId = portalId; }
		SH_USER_API auto ConsumePendingSpawnPortalId() -> int;
		SH_USER_API void SetPortalTransferCooldownMs(float ms) { portalTransferCooldownMs = ms > 0.f ? ms : 0.f; }
		SH_USER_API void SetInventory(Inventory&& inventory) noexcept { this->inventory = std::move(inventory); }
		SH_USER_API void IncreaseHeartbeat();
		SH_USER_API void Tick(float dt);

		SH_USER_API auto GetId() const -> int64_t { return id; }
		SH_USER_API auto GetIp() const -> const std::string& { return ip; }
		SH_USER_API auto GetPort() const -> uint16_t { return port; }
		SH_USER_API auto GetUserUUID() const -> const core::UUID& { return uuid; }
		SH_USER_API auto GetCurrentWorldUUID() const -> const core::UUID& { return currentWorld; }
		SH_USER_API auto GetNickName() const -> const std::string& { return nickname; }
		SH_USER_API auto GetInventory() -> Inventory& { return inventory; }
		SH_USER_API auto GetInventory() const -> const Inventory& { return inventory; }
		SH_USER_API auto GetTcpSocket() const -> network::TcpSocket* { return tcpSocket.get(); }
		SH_USER_API auto GetHeartbeat() const->uint32_t { return heartbeat; }
		SH_USER_API auto CanTransferPortal() const -> bool { return portalTransferCooldownMs <= 0.f; }
	private:
		int64_t id = 0;

		std::unique_ptr<network::TcpSocket> tcpSocket;
		std::string ip;
		uint16_t port;

		core::UUID uuid;
		core::UUID currentWorld;

		std::string nickname;

		Inventory inventory;

		uint32_t heartbeat = 10;
		float portalTransferCooldownMs = 0.f;
		int pendingSpawnPortalId = -1;
	};
}//namepsace
