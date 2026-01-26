#include "Player/PlayerPickup.h"
#include "MapleServer.h"
#include "CollisionTag.hpp"
#include "MapleWorld.h"
#include "Packet/KeyPacket.hpp"
#include "Packet/InventorySyncPacket.hpp"

#include "Game/GameObject.h"
#include "Game/Component/Phys/Collider.h"
#include "Game/Input.h"
namespace sh::game
{
	PlayerPickup::PlayerPickup(GameObject& owner) :
		Component(owner)
	{
		packetSubscriber.SetCallback(
			[this](const network::PacketEvent& evt)
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
		if (player->IsLocal())
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
			InsertItemToInventory(*item, *player);
		}
	}
	void PlayerPickup::InsertItemToInventory(Item& item, Player& player)
	{
		User* userPtr = MapleServer::GetInstance()->GetUserManager().GetUser(player.GetUserUUID());
		if (userPtr == nullptr)
			return;

		if (userPtr->GetInventory().AddItem(item.itemId, 1))
		{
			player.GetCurrentWorld()->DestroyItem(item);
			InventorySyncPacket packet{};
			packet.inventoryJson = userPtr->GetInventory().SerializeDirtySlots();
			userPtr->GetTcpSocket()->Send(packet);
		}
	}
}//namespace