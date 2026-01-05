#include "MapleWorld.h"
#include "Player/Player.h"
#include "Item.h"
#include "ItemDB.h"

#include "Packet/PlayerJoinWorldPacket.h"
#include "Packet/PlayerSpawnPacket.hpp"
#include "Packet/PlayerDespawnPacket.hpp"
#include "Packet/PlayerLeavePacket.h"
#include "Packet/HeartbeatPacket.hpp"
#include "Packet/ItemDropPacket.hpp"

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
	}
	void MapleWorld::ProcessPlayerJoin(const PlayerJoinWorldPacket& packet, const Endpoint& endpoint)
	{
		auto userPtr = server->GetUser(endpoint);
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
					const auto& playerPos = playerPtr->gameObject.transform->position;
					PlayerSpawnPacket packet;
					packet.x = playerPos.x;
					packet.y = playerPos.y;
					packet.playerUUID = playerPtr->GetUserUUID().ToString();

					server->Send(packet, endpoint.ip, endpoint.port);
				}
				// 접속한 플레이어 생성
				{
					auto player = SpawnPlayer(userPtr->GetUserUUID(), spawnPos.x, spawnPos.y);
					players[userPtr->GetUserUUID().ToString()] = player;

					PlayerSpawnPacket packet;
					packet.x = spawnPos.x;
					packet.y = spawnPos.y;
					packet.playerUUID = userPtr->GetUserUUID().ToString();

					server->BroadCast(packet);
				}
			}
		}
	}
	void MapleWorld::ProcessPlayerLeave(const PlayerLeavePacket& packet, const Endpoint& endpoint)
	{
		User* user = server->GetUser(endpoint);
		if (user == nullptr || user->GetUserUUID().ToString() != packet.playerUUID)
			return;

		PlayerDespawnPacket despawnPacket{};
		despawnPacket.playerUUID = packet.playerUUID;
		ProcessPlayerDespawn(despawnPacket);

		server->BroadCast(despawnPacket);
	}
	void MapleWorld::ProcessHeartbeat(const Endpoint& ep)
	{
		User* user = server->GetUser(ep);
		if (user == nullptr)
			return;

		auto it = players.find(user->GetUserUUID().ToString());
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
			if (!player.IsValid())
				continue;
			if (player->GetHeartbeat() <= 0)
				dead.push_back(player.Get());
		}
		for (auto player : dead)
		{
			PlayerDespawnPacket despawnPacket{};
			despawnPacket.playerUUID = player->GetUserUUID().ToString();
			ProcessPlayerDespawn(despawnPacket);

			server->BroadCast(despawnPacket);
			server->Kick(player->GetUserUUID());
		}
	}
	void MapleWorld::SpawnItem(int itemId, float x, float y, const core::UUID& owner)
	{
	}
	void MapleWorld::ProcessPlayerDespawn(const PlayerDespawnPacket& packet)
	{
		auto it = players.find(packet.playerUUID);
		if (it == players.end())
			return;

		Player* player = it->second.Get();
		players.erase(it);

		if (player == localPlayer.Get())
		{
			SH_INFO("Bye");
		}

		player->gameObject.Destroy();
	}
}//namespace