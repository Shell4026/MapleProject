#include "MapleWorld.h"
#include "Player.h"

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
				const MapleServer::Endpoint endpoint{ ip, port };

				if (evt.packet->GetId() == PlayerJoinWorldPacket::ID)
					ProcessPlayerJoin(static_cast<const PlayerJoinWorldPacket&>(*evt.packet), endpoint);
			}
		);
#else
		packetEventSubscriber.SetCallback
		(
			[&](const PacketEvent& evt)
			{
				if (evt.packet->GetId() == PlayerSpawnPacket::ID)
				{
					ProcessPlayerSpawn(static_cast<const PlayerSpawnPacket&>(*evt.packet));
				}
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
#endif
	}
	void MapleWorld::ProcessPlayerJoin(const PlayerJoinWorldPacket& packet, const MapleServer::Endpoint& endpoint)
	{
#if SH_SERVER
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
					SH_ERROR("Invalid spawn point!");
				else
				{
					const auto& pos = playerSpawnPoint->GetWorldPosition();
					SpawnPlayer(userPtr->GetUUID(), pos.x, pos.y);

					PlayerSpawnPacket packet;
					packet.x = pos.x;
					packet.y = pos.y;
					packet.playerUUID = userPtr->GetUUID().ToString();
					server->BroadCast(packet);
				}
			}
		}
#endif
	}
	void MapleWorld::ProcessPlayerSpawn(const PlayerSpawnPacket& packet)
	{
		core::UUID playerUUID{ packet.playerUUID };
		if (playerUUID == client->GetUser().GetUUID())
		{
			localPlayer = SpawnPlayer(playerUUID, packet.x, packet.y);
			camera->SetPlayer(localPlayer->gameObject);
		}
		else
			SpawnPlayer(playerUUID, packet.x, packet.y);
	}
}
