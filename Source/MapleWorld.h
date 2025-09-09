#pragma once
#include "Export.h"
#include "User.h"
#include "PacketEvent.hpp"
#include "MapleServer.h"
#include "MapleClient.h"
#include "PlayerCamera2D.h"
#include "Player.h"
#include "Packet/PlayerJoinWorldPacket.h"
#include "Packet/PlayerSpawnPacket.hpp"

#include "Core/SContainer.hpp"
#include "Core/EventSubscriber.h"

#include "Game/Component/NetworkComponent.h"
#include "Game/Component/Transform.h"
#include "Game/Prefab.h"
namespace sh::game
{
	class MapleWorld : public NetworkComponent
	{
		COMPONENT(MapleWorld, "user")
	public:
		SH_USER_API MapleWorld(GameObject& owner);

		SH_USER_API auto SpawnPlayer(const core::UUID& playerUUID, float x, float y) const -> Player*;

		SH_USER_API void Start() override;
	private:
		void ProcessPlayerJoin(const PlayerJoinWorldPacket& packet, const MapleServer::Endpoint& endpoint);

		void ProcessPlayerSpawn(const PlayerSpawnPacket& packet);
	public:
		PROPERTY(playerSpawnPoint)
		Transform* playerSpawnPoint = nullptr;
	private:
		PROPERTY(playerPrefab)
		Prefab* playerPrefab = nullptr;
		PROPERTY(camera)
		PlayerCamera2D* camera = nullptr;

		core::SObjWeakPtr<Player> localPlayer = nullptr;

		MapleServer* server = nullptr;
		MapleClient* client = nullptr;

		core::EventSubscriber<PacketEvent> packetEventSubscriber;
	};
}//namespace