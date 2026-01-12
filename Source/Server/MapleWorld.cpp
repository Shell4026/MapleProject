#include "MapleWorld.h"
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
namespace sh::game
{
	MapleWorld::MapleWorld(GameObject& owner) :
		NetworkComponent(owner)
	{
		packetEventSubscriber.SetCallback
		(
			[&](const PacketEvent& evt)
			{
				const std::string& ip = evt.senderIp;
				const uint16_t port = evt.senderPort;
				const Endpoint endpoint{ ip, port };
				uint32_t id = evt.packet->GetId();

				if (id == PlayerJoinWorldPacket::ID)
					ProcessPlayerJoin(static_cast<const PlayerJoinWorldPacket&>(*evt.packet), endpoint);
				else if (id == PlayerLeavePacket::ID)
					ProcessPlayerLeave(static_cast<const PlayerLeavePacket&>(*evt.packet), endpoint);
				else if (id == HeartbeatPacket::ID)
					ProcessHeartbeat(endpoint);
			}
		);
	}
	SH_USER_API auto MapleWorld::SpawnPlayer(const core::UUID& playerUUID, float x, float y) const -> Player*
	{
		if (playerPrefab != nullptr)
		{
			SH_INFO_FORMAT("Spawn player at {}, {}", x, y);
			auto playerObj = playerPrefab->AddToWorld(world);
			playerObj->transform->SetWorldPosition({ x, y, 0 });
			playerObj->transform->UpdateMatrix();

			auto player = playerObj->GetComponent<Player>();
			player->SetUserUUID(playerUUID);
			player->SetCurrentWorld(const_cast<MapleWorld&>(*this));

			return player;
		}
		else
			SH_ERROR("Invalid player prefab!");
		return nullptr;
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
	}
	SH_USER_API void MapleWorld::LateUpdate()
	{
		CheckHeartbeats();
		TryClearSleepItems();
		++worldTick;
	}
	SH_USER_API void MapleWorld::SpawnItem(int itemId, float x, float y, const core::UUID& owner)
	{
		SH_INFO_FORMAT("Drop item: {}", itemId);
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
			} 
			while (!core::IsValid(item) && !sleepItems.empty());

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
		auto pos = itemObj->transform->GetWorldPosition();
		pos.x = x;
		pos.y = y;
		itemObj->transform->SetWorldPosition(pos);
		
		item->GetRigidBody()->ResetPhysicsTransform();
		item->GetRigidBody()->SetLinearVelocity({ 0.f, 0.f, 0.f });
		item->instanceId = nextItemIdx;
		item->itemId = itemId;
		item->owner = owner;

		// ItemDropPacket은 MapleWorld(client)클래스에서 처리
		ItemDropPacket packet{};

		packet.itemId = itemId;
		packet.x = x;
		packet.y = y;
		packet.cnt = 1;
		packet.itemUUID = itemObj->GetUUID();
		packet.ownerUUID = owner;

		MapleServer::GetInstance()->BroadCast(packet);

		lastItemSpawnTick = worldTick;
	}
	SH_USER_API void MapleWorld::SpawnItem(const std::vector<int>& itemIds, float x, float y, const core::UUID& owner)
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

	void MapleWorld::ProcessPlayerJoin(const PlayerJoinWorldPacket& packet, const Endpoint& endpoint)
	{
		auto userPtr = server->GetUserManager().GetUser(endpoint);
		if (userPtr == nullptr)
			return;

		const auto& userWorldUUID = userPtr->GetCurrentWorldUUID();
		if (userWorldUUID.IsEmpty())
		{
			// 이 월드에 조인 했음
			if (world.GetUUID() == core::UUID{ packet.worldUUID })
			{
				if (playerSpawnPoint == nullptr)
				{
					SH_ERROR("Invalid spawn point!");
					return;
				}
				userPtr->SetCurrentWorldUUID(world.GetUUID());

				const auto& spawnPos = playerSpawnPoint->GetWorldPosition();
				// 접속한 플레이어에게 다른 플레이어 동기화
				for (auto& [uuid, playerPtr] : players)
				{
					const auto& playerPos = playerPtr->gameObject.transform->GetWorldPosition();
					PlayerSpawnPacket packet;
					packet.x = playerPos.x;
					packet.y = playerPos.y;
					packet.playerUUID = playerPtr->GetUserUUID();

					server->Send(packet, endpoint.ip, endpoint.port);
				}
				// 접속한 플레이어 생성
				{
					auto player = SpawnPlayer(userPtr->GetUserUUID(), spawnPos.x, spawnPos.y);
					players[userPtr->GetUserUUID()] = player;

					PlayerSpawnPacket packet;
					packet.x = spawnPos.x;
					packet.y = spawnPos.y;
					packet.playerUUID = userPtr->GetUserUUID();

					server->BroadCast(packet);
				}
			}
		}
	}
	void MapleWorld::ProcessPlayerLeave(const PlayerLeavePacket& packet, const Endpoint& endpoint)
	{
		User* user = server->GetUserManager().GetUser(endpoint);
		if (user == nullptr || user->GetUserUUID() != packet.playerUUID)
			return;

		PlayerDespawnPacket despawnPacket{};
		despawnPacket.playerUUID = packet.playerUUID;
		ProcessPlayerDespawn(despawnPacket);

		server->BroadCast(despawnPacket);
	}
	void MapleWorld::ProcessHeartbeat(const Endpoint& ep)
	{
		User* user = server->GetUserManager().GetUser(ep);
		if (user == nullptr)
			return;

		auto it = players.find(user->GetUserUUID());
		if (it == players.end())
			return;

		Player& player = *it->second;
		player.IncreaseHeartbeat();
	}
	void MapleWorld::CheckHeartbeats()
	{
		std::vector<Player*> dead{};
		for (auto& [uuidStr, player] : players)
		{
			if (!core::IsValid(player))
				continue;
			if (player->GetHeartbeat() <= 0)
				dead.push_back(player);
		}
		for (auto player : dead)
		{
			PlayerDespawnPacket despawnPacket{};
			despawnPacket.playerUUID = player->GetUserUUID();
			ProcessPlayerDespawn(despawnPacket);

			server->BroadCast(despawnPacket);
			server->GetUserManager().KickUser(player->GetUserUUID());
		}
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
	void MapleWorld::ProcessPlayerDespawn(const PlayerDespawnPacket& packet)
	{
		auto it = players.find(packet.playerUUID);
		if (it == players.end())
			return;

		Player* player = it->second;
		players.erase(it);

		player->gameObject.Destroy();
	}
}//namespace