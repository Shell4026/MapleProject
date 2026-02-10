#pragma once
#include "Export.h"
#include "User.h"
#include "EndPoint.hpp"
#include "MapleServer.h"
#include "MapleClient.h"
#include "Player/PlayerCamera2D.h"
#include "Item/Item.h"
#include "Physics/Foothold.h"

#include "Core/SContainer.hpp"
#include "Core/EventSubscriber.h"

#include "Network/PacketEvent.hpp"

#include "Game/Component/Component.h"
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
	class MapleWorld : public Component
	{
		COMPONENT(MapleWorld, "user")
	public:
		SH_USER_API MapleWorld(GameObject& owner);

		SH_USER_API auto SpawnPlayer(const core::UUID& userUUID, float x, float y) -> Player*;
		SH_USER_API auto DespawnPlayer(const core::UUID& userUUID) -> bool;

		SH_USER_API void Awake() override;
		SH_USER_API void Start() override;
		SH_USER_API void LateUpdate() override;

		SH_USER_API auto GetWorldTick() const -> uint64_t { return worldTick; }
		SH_USER_API auto GetFoothold() const -> Foothold* { return foothold; }
#if SH_SERVER
		SH_USER_API void SpawnItem(int itemId, float x, float y, const core::UUID& owner);
		SH_USER_API void SpawnItem(const std::vector<int>& itemIds, float x, float y, const core::UUID& owner);
		SH_USER_API void DestroyItem(Item& item);
		/// @brief 월드에 존재하는 메이플 월드 컴포넌트를 가져오는 함수
		/// @param worldUUID 월드 UUID (메이플 월드 UUID가 아님!)
		/// @return 월드에 없으면 nullptr
		SH_USER_API static auto GetMapleWorld(const core::UUID& worldUUID) -> MapleWorld*;
#endif
	private:
#if SH_SERVER
		void ProcessPlayerJoin(const PlayerJoinWorldPacket& packet);
		void ProcessPlayerLeave(const PlayerLeavePacket& packet);
		void TryClearSleepItems();
#else
		void ProcessPlayerSpawn(const PlayerSpawnPacket& packet);
		void ProcessItemDrop(const ItemDropPacket& packet);
		void ProcessItemDespawn(const ItemDespawnPacket& packet);
#endif
	public:
		PROPERTY(playerSpawnPoint)
		Transform* playerSpawnPoint = nullptr;
	private:
		PROPERTY(foothold)
		Foothold* foothold = nullptr;
		PROPERTY(playerPrefab)
		Prefab* playerPrefab = nullptr;
		PROPERTY(itemPrefab)
		Prefab* itemPrefab = nullptr;

		std::unordered_map<core::UUID, Player*> players;
		std::queue<core::SObjWeakPtr<Item>> sleepItems;

		core::EventSubscriber<network::PacketEvent> packetEventSubscriber;

		uint64_t worldTick = 0;
#if SH_SERVER
		MapleServer* server = nullptr;
		uint64_t nextItemIdx = 0;
		uint64_t clearSleepItemsAfterTicks = 600;
		uint64_t lastItemSpawnTick = 0;

		static std::unordered_map<core::UUID, MapleWorld*> mapleWorlds;
#else
		PROPERTY(camera)
		PlayerCamera2D* camera = nullptr;
		MapleClient* client = nullptr;
#endif
	};
}//namespace