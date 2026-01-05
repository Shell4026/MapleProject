#pragma once
#include "Export.h"
#include "User.h"
#include "EndPoint.hpp"
#include "PacketEvent.hpp"
#include "MapleServer.h"
#include "MapleClient.h"
#include "Player/PlayerCamera2D.h"
#include "Player/Player.h"
#include "Mob/MobSpawner.h"

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
	class ItemDropPacket;

	/// @brief 맵의 단위
	class MapleWorld : public NetworkComponent
	{
		COMPONENT(MapleWorld, "user")
	public:
		SH_USER_API MapleWorld(GameObject& owner);

		SH_USER_API auto SpawnPlayer(const core::UUID& playerUUID, float x, float y) const -> Player*;

		SH_USER_API void Awake() override;
		SH_USER_API void Start() override;
		SH_USER_API void LateUpdate() override;

		SH_USER_API auto GetMobSpawner() const -> MobSpawner* { return mobSpawner; }
	private:
#if SH_SERVER
		void ProcessPlayerJoin(const PlayerJoinWorldPacket& packet, const Endpoint& endpoint);
		void ProcessPlayerLeave(const PlayerLeavePacket& packet, const Endpoint& endpoint);
		void ProcessHeartbeat(const Endpoint& ep);
		void CheckHeartbeats();
#else
		void ProcessPlayerSpawn(const PlayerSpawnPacket& packet, const Endpoint& endpoint);
		void ProcessItemDrop(const ItemDropPacket& packet);
#endif
		void ProcessPlayerDespawn(const PlayerDespawnPacket& packet);
	public:
		PROPERTY(playerSpawnPoint)
		Transform* playerSpawnPoint = nullptr;
	private:
		PROPERTY(playerPrefab)
		Prefab* playerPrefab = nullptr;
		PROPERTY(itemPrefab)
		Prefab* itemPrefab = nullptr;
		PROPERTY(mobSpawner)
		MobSpawner* mobSpawner = nullptr;

		core::SObjWeakPtr<Player> localPlayer = nullptr;
		std::unordered_map<std::string, core::SObjWeakPtr<Player>> players; // key = uuid

#if SH_SERVER
		MapleServer* server = nullptr;
#else
		PROPERTY(camera)
		PlayerCamera2D* camera = nullptr;
		MapleClient* client = nullptr;
#endif

		core::EventSubscriber<PacketEvent> packetEventSubscriber;
	};
}//namespace