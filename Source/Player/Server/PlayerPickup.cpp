#include "Player/PlayerPickup.h"
#include "Packet/KeyPacket.hpp"
#include "MapleServer.h"
#include "CollisionTag.hpp"


#include "Game/GameObject.h"
#include "Game/Component/Collider.h"
#include "Game/Input.h"
namespace sh::game
{
	PlayerPickup::PlayerPickup(GameObject& owner) :
		Component(owner)
	{
		packetSubscriber.SetCallback(
			[this](const PacketEvent& evt)
			{
				if (player == nullptr)
					return;
				if (evt.packet->GetId() == KeyPacket::ID)
				{
					auto keyPacket = static_cast<const KeyPacket*>(evt.packet);

					if (player->GetUserUUID() != keyPacket->userUUID)
						return;

					if (keyPacket->keycode == static_cast<int>(Input::KeyCode::Z))
					{
						if (keyPacket->bPressed)
							bPickupState = true;
						else
							bPickupState = false;
					}
				}
			}
		);
	}
	SH_USER_API void PlayerPickup::Awake()
	{
		player = gameObject.GetComponent<Player>();
		if (player == nullptr)
		{
			SH_ERROR("Not found Player component!");
		}
		MapleServer::GetInstance()->bus.Subscribe(packetSubscriber);
	}
	SH_USER_API void PlayerPickup::BeginUpdate()
	{
	}
	SH_USER_API void PlayerPickup::Update()
	{
		if (bPickupState)
			ProcessPickup();
	}
	SH_USER_API void PlayerPickup::OnTriggerEnter(Collider& collider)
	{
		if (collider.GetCollisionTag() != tag::itemTag)
			return;

		auto& itemObj = collider.gameObject.transform->GetParent()->gameObject;
		Item* item = itemObj.GetComponent<Item>();
		if (core::IsValid(item))
			hitItems[item->instanceId] = item;
	}
	SH_USER_API void PlayerPickup::OnTriggerExit(Collider& collider)
	{
		if (collider.GetCollisionTag() != tag::itemTag)
			return;

		auto& itemObj = collider.gameObject.transform->GetParent()->gameObject;
		Item* item = itemObj.GetComponent<Item>();
		if (core::IsValid(item))
		{
			auto it = hitItems.find(item->instanceId);
			if (it != hitItems.end())
				hitItems.erase(it);
		}
	}
	void PlayerPickup::ProcessPickup()
	{
		if (player == nullptr)
			return;
		for (const auto& [instanceId, item] : hitItems)
		{
			if (!core::IsValid(item))
				continue;
			InsertItemToDB(MapleServer::GetInstance()->GetDB(), *item, *player);
		}
	}
	void PlayerPickup::InsertItemToDB(Database& db, Item& item, Player& player)
	{
		static Database::SQL insertSQL = db.CreateSQL("INSERT INTO UserInventory (itemId, ownerId, slotIdx, count) VALUES (?, ?, ?, ?);");
		if (!db.Execute("BEGIN TRANSACTION;"))
			SH_ERROR("BEGIN TRANSACTION;");
		if (db.Execute(insertSQL, item.itemId, 0, 0, 1))
		{
			if (db.Execute("COMMIT;"))
			{
				SH_INFO_FORMAT("User: {}, Item: {}", player.GetUserUUID().ToString(), item.itemId);
				item.gameObject.Destroy();
			}
		}
		else
		{
			SH_ERROR("Rollback;");
			db.Execute("ROLLBACK;");
		}
	}
}//namespace