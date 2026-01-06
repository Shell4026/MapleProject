#pragma once
#include "Export.h"
#include "User.h"
#include "EndPoint.hpp"
#include "PacketEvent.hpp"
#include "MapleServer.h"
#include "MapleClient.h"
#include "Player/PlayerCamera2D.h"
#include "Player/Player.h"
#include "Item/Item.h"

#include "Core/SContainer.hpp"
#include "Core/EventSubscriber.h"

#include "Game/Component/NetworkComponent.h"
#include "Game/Component/Transform.h"
#include "Game/Prefab.h"

#include <unordered_map>

#include <cstdint>
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

#if SH_SERVER
		SH_USER_API void SpawnItem(int itemId, float x, float y, const core::UUID& owner);
		SH_USER_API void SpawnItem(const std::vector<int>& itemIds, float x, float y, const core::UUID& owner);
		SH_USER_API void DestroyItem(Item& item);
#endif
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

		core::SObjWeakPtr<Player> localPlayer = nullptr;
		PROPERTY(players)
		std::unordered_map<core::UUID, Player*> players; // key = uuid
		std::queue<core::SObjWeakPtr<Item>> sleepItems;

		core::EventSubscriber<PacketEvent> packetEventSubscriber;
#if SH_SERVER
		MapleServer* server = nullptr;
		uint64_t nextItemIdx = 0;
#else
		PROPERTY(camera)
		PlayerCamera2D* camera = nullptr;
		MapleClient* client = nullptr;
#endif
	};
}//namespace