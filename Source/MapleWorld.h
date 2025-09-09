#pragma once
#include "Export.h"
#include "User.h"
#include "EndPoint.hpp"
#include "PacketEvent.hpp"
#include "MapleServer.h"
#include "MapleClient.h"
#include "PlayerCamera2D.h"
#include "Player.h"

#include "Core/SContainer.hpp"
#include "Core/EventSubscriber.h"

#include "Game/Component/NetworkComponent.h"
#include "Game/Component/Transform.h"
#include "Game/Prefab.h"

#include <unordered_map>
namespace sh::game
{
	class PlayerJoinWorldPacket;
	class PlayerLeavePacket;
	class PlayerSpawnPacket;
	class PlayerDespawnPacket;

	class MapleWorld : public NetworkComponent
	{
		COMPONENT(MapleWorld, "user")
	public:
		SH_USER_API MapleWorld(GameObject& owner);

		SH_USER_API auto SpawnPlayer(const core::UUID& playerUUID, float x, float y) const -> Player*;

		SH_USER_API void Start() override;
		SH_USER_API void LateUpdate() override;
	private:
#if SH_SERVER
		void ProcessPlayerJoin(const PlayerJoinWorldPacket& packet, const Endpoint& endpoint);
		void ProcessPlayerLeave(const PlayerLeavePacket& packet, const Endpoint& endpoint);
		void ProcessHeartbeat(const Endpoint& ep);
		void CheckHeartbeats();
#else
		void ProcessPlayerSpawn(const PlayerSpawnPacket& packet, const Endpoint& endpoint);
#endif
		void ProcessPlayerDespawn(const PlayerDespawnPacket& packet);
	public:
		PROPERTY(playerSpawnPoint)
		Transform* playerSpawnPoint = nullptr;
	private:
		PROPERTY(playerPrefab)
		Prefab* playerPrefab = nullptr;
		PROPERTY(camera)
		PlayerCamera2D* camera = nullptr;

		core::SObjWeakPtr<Player> localPlayer = nullptr;
		std::unordered_map<std::string, core::SObjWeakPtr<Player>> players; // key = uuid

#if SH_SERVER
		MapleServer* server = nullptr;
#else
		MapleClient* client = nullptr;
#endif

		core::EventSubscriber<PacketEvent> packetEventSubscriber;
	};
}//namespace