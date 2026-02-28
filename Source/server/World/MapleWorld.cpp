#include "World/MapleWorld.h"
#include "Phys/FootholdMovement.h"
#include "Player/Player.h"
#include "Item/Item.h"

#include "Packet/PlayerJoinWorldPacket.hpp"
#include "Packet/PlayerSpawnPacket.hpp"
#include "Packet/PlayerDespawnPacket.hpp"
#include "Packet/PlayerLeavePacket.hpp"
#include "Packet/HeartbeatPacket.hpp"
#include "Packet/ItemDropPacket.hpp"
#include "Packet/ItemDespawnPacket.hpp"

#include "Game/World.h"
#include "Game/GameObject.h"
// 서버 사이드
namespace sh::game
{
	std::unordered_map<core::UUID, MapleWorld*> MapleWorld::mapleWorlds;

	MapleWorld::MapleWorld(GameObject& owner) :
		Component(owner)
	{
		packetEventSubscriber.SetCallback
		(
			[&](const network::PacketEvent& evt)
			{
				const std::string& ip = evt.senderIp;
				const uint16_t port = evt.senderPort;
				const Endpoint endpoint{ ip, port };
				uint32_t id = evt.packet->GetId();

				if (id == PlayerJoinWorldPacket::ID)
					ProcessPlayerJoin(static_cast<const PlayerJoinWorldPacket&>(*evt.packet));
			}
		);
		mapleWorlds[world.GetUUID()] = this;
	}
	SH_USER_API auto MapleWorld::SpawnPlayer(const core::UUID& uuid, float x, float y) -> Player*
	{
		if (playerPrefab != nullptr)
		{
			SH_INFO_FORMAT("Spawn player at {}, {}", x, y);
			auto playerObj = playerPrefab->AddToWorld(world);
			playerObj->transform->SetWorldPosition({ x, y, 0 });
			playerObj->transform->UpdateMatrix();

			Player* const player = playerObj->GetComponent<Player>();
			player->SetUserUUID(uuid, Player::MapleWorldKey{});
			player->SetCurrentWorld(*this);
			players[uuid] = player;

			router.RegisterPlayer(*player);

			return player;
		}
		else
			SH_ERROR("Invalid player prefab!");
		return nullptr;
	}
	SH_USER_API auto MapleWorld::DespawnPlayer(const core::UUID& uuid) -> bool
	{
		auto it = players.find(uuid);
		if (it == players.end())
			return false;

		Player* player = it->second;
		players.erase(it);

		player->gameObject.Destroy();

		PlayerDespawnPacket packet{};
		packet.player = player->GetUUID();

		server->BroadCast(packet);

		router.UnRegisterPlayer(player);

		return true;
	}
	SH_USER_API void MapleWorld::Awake()
	{
		if (itemPrefab == nullptr)
		{
			itemPrefab = static_cast<Prefab*>(core::SObject::GetSObjectUsingResolver(core::UUID{ Item::PREFAB_UUID }));
			if (itemPrefab == nullptr)
				SH_ERROR_FORMAT("Item prefab is not valid!: {}", Item::PREFAB_UUID);
			else
			{
				core::GarbageCollection::GetInstance()->SetRootSet(itemPrefab);
			}
		}
	}
	SH_USER_API void MapleWorld::Start()
	{
		server = MapleServer::GetInstance();
		if (server == nullptr)
			throw std::runtime_error{ "Invalid server" };

		server->bus.Subscribe(packetEventSubscriber);
		router.SetPacketBus(server->bus);
	}
	SH_USER_API void MapleWorld::LateUpdate()
	{
		TryClearSleepItems();
		++worldTick;
	}
	SH_USER_API void MapleWorld::SpawnItem(int itemId, float x, float y, const Player* owner)
	{
		SH_INFO_FORMAT("Drop item: {}", itemId);
		Item& item = GetEmptyItem();
		GameObject& itemObj = item.gameObject;

		auto pos = itemObj.transform->GetWorldPosition();
		pos.x = x;
		pos.y = y;
		itemObj.transform->SetWorldPosition(pos);
		
		item.GetMovement()->AddImpulse(0.f, 6.f);
		item.instanceId = nextItemIdx;
		item.itemId = itemId;
		if (owner != nullptr)
			item.owner = owner->GetUserUUID();

		// ItemDropPacket은 MapleWorld(client)클래스에서 처리
		ItemDropPacket packet{};

		packet.itemId = itemId;
		packet.x = x;
		packet.y = y;
		packet.cnt = 1;
		packet.itemUUID = itemObj.GetUUID();
		if (owner != nullptr)
			packet.ownerUUID = owner->GetUUID();

		MapleServer::GetInstance()->BroadCast(packet);

		lastItemSpawnTick = worldTick;
	}
	SH_USER_API void MapleWorld::SpawnItem(const std::vector<int>& itemIds, float x, float y, const Player* owner)
	{
		for (int id : itemIds)
			SpawnItem(id, x, y, owner);
	}

	SH_USER_API void MapleWorld::DestroyItem(Item& item)
	{
		item.gameObject.SetActive(false);
		sleepItems.push(&item);

		ItemDespawnPacket packet{};
		packet.itemObjectUUID = item.gameObject.GetUUID();

		server->BroadCast(packet);
	}

	SH_USER_API auto MapleWorld::GetMapleWorld(const core::UUID& worldUUID) -> MapleWorld*
	{
		auto it = mapleWorlds.find(worldUUID);
		if (it == mapleWorlds.end())
			return nullptr;
		return it->second;
	}

	void MapleWorld::ProcessPlayerJoin(const PlayerJoinWorldPacket& packet)
	{
		if (world.GetUUID() != packet.worldUUID)
			return;

		const User* const userPtr = server->GetUserManager().GetUser(packet.user);
		if (userPtr == nullptr)
			return;

		const auto& userWorldUUID = userPtr->GetCurrentWorldUUID();
		if (userWorldUUID != world.GetUUID())
		{
			SH_ERROR_FORMAT("User({})'s current world is diffrent", userPtr->GetUserUUID().ToString());
			return;
		}

		if (playerSpawnPoint == nullptr)
		{
			SH_ERROR("Invalid spawn point!");
			return;
		}

		const auto& spawnPos = playerSpawnPoint->GetWorldPosition();
		// 접속한 플레이어에게 다른 플레이어 동기화
		for (const auto& [uuid, playerPtr] : players)
		{
			const User* const remoteUserPtr = server->GetUserManager().GetUser(playerPtr->GetUserUUID());

			const auto& playerPos = playerPtr->gameObject.transform->GetWorldPosition();
			PlayerSpawnPacket spawnPacket;
			spawnPacket.x = playerPos.x;
			spawnPacket.y = playerPos.y;
			spawnPacket.playerUUID = playerPtr->GetUUID();
			spawnPacket.nickname = remoteUserPtr->GetNickName();
			spawnPacket.bLocal = false;

			server->Send(spawnPacket, userPtr->GetIp(), userPtr->GetPort());
		}
		// 접속한 플레이어 생성
		{
			const Player* const player = SpawnPlayer(userPtr->GetUserUUID(), spawnPos.x, spawnPos.y);

			PlayerSpawnPacket spawnPacket;
			spawnPacket.x = spawnPos.x;
			spawnPacket.y = spawnPos.y;
			spawnPacket.playerUUID = player->GetUUID();
			spawnPacket.nickname = userPtr->GetNickName();
			spawnPacket.bLocal = false;
			server->BroadCast(spawnPacket, Endpoint{ userPtr->GetIp(), userPtr->GetPort() });
			spawnPacket.bLocal = true;
			server->Send(spawnPacket, userPtr->GetIp(), userPtr->GetPort());
		}
	}
	void MapleWorld::ProcessPlayerLeave(const PlayerLeavePacket& packet)
	{
		User* user = server->GetUserManager().GetUser(packet.user);
		if (user == nullptr)
			return;

		if (user->GetCurrentWorldUUID() != world.GetUUID())
			return;

		DespawnPlayer(packet.user);

		PlayerDespawnPacket despawnPacket{};
		despawnPacket.player = packet.user;
		server->BroadCast(despawnPacket);
	}
	auto MapleWorld::GetEmptyItem() -> Item&
	{
		GameObject* itemObj = nullptr;
		Item* item = nullptr;
		if (sleepItems.empty())
		{
			itemObj = itemPrefab->AddToWorld(world);
			item = itemObj->GetComponent<Item>();
		}
		else
		{
			do
			{
				item = sleepItems.front().Get();
				sleepItems.pop();
			} while (!core::IsValid(item) && !sleepItems.empty());

			if (!core::IsValid(item))
			{
				itemObj = itemPrefab->AddToWorld(world);
				item = itemObj->GetComponent<Item>();
			}
			else
			{
				itemObj = &item->gameObject;
				itemObj->SetUUID(core::UUID::Generate());
				itemObj->SetActive(true);
			}
		}

		item->SetCurrentWorld(*this);

		return *item;
	}
	void MapleWorld::TryClearSleepItems()
	{
		if (sleepItems.empty())
			return;

		const uint64_t noSpawnTicks = worldTick - lastItemSpawnTick;
		if (noSpawnTicks < clearSleepItemsAfterTicks)
			return;

		while (!sleepItems.empty())
		{
			Item* item = sleepItems.front().Get();
			sleepItems.pop();

			if (!core::IsValid(item))
				continue;

			item->gameObject.Destroy();
		}
		sleepItems = std::queue<core::SObjWeakPtr<Item>>();
		SH_INFO("Clear sleepItems...");
	}
}//namespace