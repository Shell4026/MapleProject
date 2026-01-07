#pragma once
#include "Export.h"
#include "User.h"
#include "EndPoint.hpp"
#include "PacketEvent.hpp"
#include "MapleServer.h"
#include "MapleClient.h"
#include "Player/PlayerCamera2D.h"
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
	class Player;
	class PlayerJoinWorldPacket;
	class PlayerLeavePacket;
	class PlayerSpawnPacket;
	class PlayerDespawnPacket;
	class ItemDropPacket;
	class ItemDespawnPacket;

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

		SH_USER_API auto GetWorldTick() const -> uint64_t { return worldTick; }
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
		void TryClearSleepItems();
#else
		void ProcessPlayerSpawn(const PlayerSpawnPacket& packet, const Endpoint& endpoint);
		void ProcessItemDrop(const ItemDropPacket& packet);
		void ProcessItemDespawn(const ItemDespawnPacket& packet);
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

		std::unordered_map<core::UUID, Player*> players;
		std::queue<core::SObjWeakPtr<Item>> sleepItems;

		core::EventSubscriber<PacketEvent> packetEventSubscriber;

		uint64_t worldTick = 0;
#if SH_SERVER
		MapleServer* server = nullptr;
		uint64_t nextItemIdx = 0;
		uint64_t clearSleepItemsAfterTicks = 600;
		uint64_t lastItemSpawnTick = 0;
		
#else
		PROPERTY(camera)
		PlayerCamera2D* camera = nullptr;
		MapleClient* client = nullptr;
#endif
	};
}//namespace