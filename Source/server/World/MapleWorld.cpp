#include "World/MapleWorld.h"
#include "Phys/FootholdMovement.h"
#include "Player/Player.h"
#include "World/Portal.h"
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

#include <algorithm>
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

		BroadCastToWorld(packet);

		router.UnRegisterPlayer(player);

		return true;
	}
	SH_USER_API void MapleWorld::BroadCastToWorld(const network::Packet& packet)
	{
		if (server == nullptr)
			return;

		for (const auto& [userUUID, player] : players)
		{
			(void)player;

			const User* const userPtr = server->GetUserManager().GetUser(userUUID);
			if (userPtr == nullptr)
				continue;

			server->Send(packet, userPtr->GetIp(), userPtr->GetPort());
		}
	}
	SH_USER_API void MapleWorld::BroadCastToWorld(const network::Packet& packet, const core::UUID& ignoreUserUUID)
	{
		if (server == nullptr)
			return;

		for (const auto& [userUUID, player] : players)
		{
			if (userUUID == ignoreUserUUID)
				continue;

			const User* const userPtr = server->GetUserManager().GetUser(userUUID);
			if (userPtr == nullptr)
				continue;

			server->Send(packet, userPtr->GetIp(), userPtr->GetPort());
		}
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

		BroadCastToWorld(packet);

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

		BroadCastToWorld(packet);
	}
	SH_USER_API void MapleWorld::RegisterPortal(Portal& portal)
	{
		for (Portal* registeredPortal : portals)
		{
			if (registeredPortal == &portal)
				return;
		}
		portals.push_back(&portal);
	}
	SH_USER_API void MapleWorld::UnRegisterPortal(Portal* portal)
	{
		if (portal == nullptr)
			return;

		auto it = std::remove(portals.begin(), portals.end(), portal);
		if (it != portals.end())
			portals.erase(it, portals.end());
	}
	SH_USER_API auto MapleWorld::TryTransferByPortal(Player& player) -> bool
	{
		for (Portal* portal : portals)
		{
			if (portal != nullptr && portal->TryTransfer(player))
				return true;
		}
		return false;
	}
	auto MapleWorld::FindPortal(int portalId) const -> Portal*
	{
		for (Portal* portal : portals)
		{
			if (portal != nullptr && portal->GetPortalId() == portalId)
				return portal;
		}
		return nullptr;
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

		User* const userPtr = server->GetUserManager().GetUser(packet.user);
		if (userPtr == nullptr)
			return;

		if (userPtr->GetCurrentWorldUUID() != packet.worldUUID)
		{
			SH_ERROR_FORMAT("Wrong world move(user: {} world: {})", userPtr->GetUserUUID().ToString(), core::UUID{ packet.worldUUID }.ToString());
			return;
		}

		if (playerSpawnPoint == nullptr)
		{
			SH_ERROR("Invalid spawn point!");
			return;
		}

		auto spawnPos = playerSpawnPoint->GetWorldPosition();
		const int spawnPortalId = userPtr->ConsumePendingSpawnPortalId();
		if (spawnPortalId >= 0)
		{
			if (Portal* const portal = FindPortal(spawnPortalId); portal != nullptr)
				spawnPos = portal->gameObject.transform->GetWorldPosition();
		}
		// 접속한 플레이어에게 다른 플레이어 동기화
		for (const auto& [uuid, playerPtr] : players)
		{
			const User* const remoteUserPtr = server->GetUserManager().GetUser(playerPtr->GetUserUUID());
			if (remoteUserPtr == nullptr)
				continue;

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
			BroadCastToWorld(spawnPacket, userPtr->GetUserUUID());
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
		BroadCastToWorld(despawnPacket);
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
