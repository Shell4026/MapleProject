#include "MapleWorld.h"
#include "Player.h"
#include "Packet/PlayerJoinWorldPacket.h"
#include "Packet/PlayerSpawnPacket.hpp"
#include "Packet/PlayerDespawnPacket.hpp"
#include "Packet/PlayerLeavePacket.h"
#include "Packet/HeartbeatPacket.hpp"

#include "Game/World.h"
#include "Game/GameObject.h"
namespace sh::game
{
	MapleWorld::MapleWorld(GameObject& owner) :
		NetworkComponent(owner)
	{
#if SH_SERVER
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
#else
		packetEventSubscriber.SetCallback
		(
			[&](const PacketEvent& evt)
			{
				Endpoint ep{ evt.senderIp, evt.senderPort };
				if (evt.packet->GetId() == PlayerSpawnPacket::ID)
					ProcessPlayerSpawn(static_cast<const PlayerSpawnPacket&>(*evt.packet), ep);
				else if (evt.packet->GetId() == PlayerDespawnPacket::ID)
					ProcessPlayerDespawn(static_cast<const PlayerDespawnPacket&>(*evt.packet));
			}
		);
#endif
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
	SH_USER_API void MapleWorld::Start()
	{
#if SH_SERVER
		server = MapleServer::GetInstance();
		if (server == nullptr)
			throw std::runtime_error{ "Invalid server" };

		server->bus.Subscribe(packetEventSubscriber);
#else
		client = MapleClient::GetInstance();
		assert(client != nullptr);

		client->bus.Subscribe(packetEventSubscriber);

		PlayerJoinWorldPacket packet{};
		packet.worldUUID = GetUUID().ToString();

		client->SendPacket(packet);
		SH_INFO_FORMAT("Join the world {}", world.GetUUID().ToString());
#endif
	}
	SH_USER_API void MapleWorld::LateUpdate()
	{
#if SH_SERVER
		CheckHeartbeats();
#endif
	}
#if SH_SERVER
	void MapleWorld::ProcessPlayerJoin(const PlayerJoinWorldPacket& packet, const Endpoint& endpoint)
	{
		auto userPtr = server->GetUser(endpoint);
		if (userPtr == nullptr)
			return;

		const auto& userWorldUUID = userPtr->GetCurrentWorldUUID();
		if (userWorldUUID.IsEmpty())
		{
			// 이 월드에 조인 했음
			if (GetUUID() == core::UUID{ packet.worldUUID })
			{
				if (playerSpawnPoint == nullptr)
				{
					SH_ERROR("Invalid spawn point!");
					return;
				}
				userPtr->SetCurrentWorldUUID(GetUUID());

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
#else
	void MapleWorld::ProcessPlayerSpawn(const PlayerSpawnPacket& packet, const Endpoint& endpoint)
	{
		core::UUID playerUUID{ packet.playerUUID };

		Player* player = nullptr;
		player = SpawnPlayer(playerUUID, packet.x, packet.y);

		if (playerUUID == client->GetUser().GetUserUUID())
		{
			localPlayer = player;
			camera->SetPlayer(localPlayer->gameObject);
		}

		players.insert_or_assign(packet.playerUUID, player);
	}
#endif
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