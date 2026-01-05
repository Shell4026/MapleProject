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
				Endpoint ep{ evt.senderIp, evt.senderPort };
				if (evt.packet->GetId() == PlayerSpawnPacket::ID)
					ProcessPlayerSpawn(static_cast<const PlayerSpawnPacket&>(*evt.packet), ep);
				else if (evt.packet->GetId() == PlayerDespawnPacket::ID)
					ProcessPlayerDespawn(static_cast<const PlayerDespawnPacket&>(*evt.packet));
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
		client = MapleClient::GetInstance();
		assert(client != nullptr);

		client->bus.Subscribe(packetEventSubscriber);

		PlayerJoinWorldPacket packet{};
		packet.worldUUID = world.GetUUID().ToString();

		client->SendPacket(packet);
		SH_INFO_FORMAT("Join the world {}", world.GetUUID().ToString());
	}
	SH_USER_API void MapleWorld::LateUpdate()
	{
	}
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
	void MapleWorld::ProcessItemDrop(const ItemDropPacket& packet)
	{
		if (itemPrefab == nullptr)
		{
			SH_ERROR("Item prefab is not valid!");
			return;
		}

		const core::Json* itemInfo = ItemDB::GetInstance()->GetItemInfo(packet.itemId);
		if (itemInfo == nullptr)
		{
			SH_ERROR_FORMAT("Item {} is not valid!", packet.itemId);
			return;
		}

		SH_INFO_FORMAT("Drop item: {}", packet.itemId);
		GameObject* itemObj = itemPrefab->AddToWorld(world);
		itemObj->SetUUID(core::UUID{ packet.itemUUID });
		auto pos = itemObj->transform->GetWorldPosition();
		pos.x = packet.x;
		pos.y = packet.y;
		itemObj->transform->SetWorldPosition(pos);

		Item* item = itemObj->GetComponent<Item>();
		item->itemId = packet.itemId;
		item->GetRigidBody()->ResetPhysicsTransform();
		item->GetRigidBody()->SetLinearVelocity({ 0.f, 5.f, 0.f });

		if (!itemInfo->contains("tex"))
			return;

		const std::string& texUUID = (*itemInfo)["tex"];
		auto texPtr = static_cast<render::Texture*>(core::SObject::GetSObjectUsingResolver(core::UUID{ texUUID }));
		if (texPtr == nullptr)
		{
			SH_ERROR_FORMAT("texture {} is not valid!", texUUID);
			return;
		}
		item->SetTexture(texPtr);
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