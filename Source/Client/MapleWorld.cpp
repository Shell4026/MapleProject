#include "MapleWorld.h"
#include "Player/Player.h"
#include "Item/Item.h"
#include "Item/ItemDB.h"

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
		Component(owner)
	{
		packetEventSubscriber.SetCallback
		(
			[&](const network::PacketEvent& evt)
			{
				Endpoint ep{ evt.senderIp, evt.senderPort };
				if (evt.packet->GetId() == PlayerSpawnPacket::ID)
					ProcessPlayerSpawn(static_cast<const PlayerSpawnPacket&>(*evt.packet));
				else if (evt.packet->GetId() == PlayerDespawnPacket::ID)
					DespawnPlayer(static_cast<const PlayerDespawnPacket&>(*evt.packet).user);
				else if (evt.packet->GetId() == ItemDropPacket::ID)
					ProcessItemDrop(static_cast<const ItemDropPacket&>(*evt.packet));
				else if (evt.packet->GetId() == ItemDespawnPacket::ID)
					ProcessItemDespawn(static_cast<const ItemDespawnPacket&>(*evt.packet));
			}
		);
	}
	SH_USER_API auto MapleWorld::SpawnPlayer(const core::UUID& userUUID, float x, float y) -> Player*
	{
		if (playerPrefab != nullptr)
		{
			SH_INFO_FORMAT("Spawn player at {}, {}", x, y);
			auto playerObj = playerPrefab->AddToWorld(world);
			playerObj->transform->SetWorldPosition({ x, y, 0 });
			playerObj->transform->UpdateMatrix();

			auto player = playerObj->GetComponent<Player>();
			player->SetCurrentWorld(*this);
			player->SetUserUUID(userUUID);
			players[userUUID] = player;

			return player;
		}
		else
			SH_ERROR("Invalid player prefab!");
		return nullptr;
	}
	SH_USER_API auto MapleWorld::DespawnPlayer(const core::UUID& userUUID) -> bool
	{
		auto it = players.find(userUUID);
		if (it == players.end())
			return false;

		Player* player = it->second;
		players.erase(it);

		if (player->IsLocal())
		{
			SH_INFO("Bye");
		}

		player->gameObject.Destroy();
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
		client = MapleClient::GetInstance();
		assert(client != nullptr);

		client->bus.Subscribe(packetEventSubscriber);

		PlayerJoinWorldPacket packet{};
		packet.worldUUID = world.GetUUID();
		packet.user = client->GetUser().GetUserUUID();

		client->SendTcp(packet);
		SH_INFO_FORMAT("Join the world {}", world.GetUUID().ToString());
	}
	SH_USER_API void MapleWorld::LateUpdate()
	{
	}

	void MapleWorld::ProcessPlayerSpawn(const PlayerSpawnPacket& packet)
	{
		core::UUID playerUUID{ packet.playerUUID };

		Player* player = nullptr;
		player = SpawnPlayer(playerUUID, packet.x, packet.y);
		player->GetNameTag()->SetNameStr(packet.nickname);
		if (playerUUID == client->GetUser().GetUserUUID())
			camera->SetPlayer(player->gameObject);
	}
	void MapleWorld::ProcessItemDrop(const ItemDropPacket& packet)
	{
		if (itemPrefab == nullptr)
		{
			SH_ERROR("Item prefab is not valid!");
			return;
		}

		const ItemInfo* itemInfo = ItemDB::GetInstance()->GetItemInfo(packet.itemId);
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

		auto texPtr = static_cast<render::Texture*>(core::SObject::GetSObjectUsingResolver(itemInfo->texUUID));
		if (texPtr == nullptr)
		{
			SH_ERROR_FORMAT("texture {} is not valid!", itemInfo->texUUID.ToString());
			return;
		}
		item->SetTexture(texPtr);
	}
	void MapleWorld::ProcessItemDespawn(const ItemDespawnPacket& packet)
	{
		core::UUID itemObjUUID = packet.itemObjectUUID;
		SH_INFO_FORMAT("Despawn: {}", itemObjUUID.ToString());
		auto obj = core::SObjectManager::GetInstance()->GetSObject(itemObjUUID);
		if (!core::IsValid(obj))
			return;
		obj->Destroy();
	}
}//namespace