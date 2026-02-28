#include "Player/PlayerPickup.h"
#include "Player/Player.h"
#include "World/MapleServer.h"
#include "World/MapleWorld.h"
#include "Phys/CollisionTag.hpp"
#include "Packet/InventorySyncPacket.hpp"

#include "Game/GameObject.h"
#include "Game/Component/Phys/Collider.h"
// 서버 사이드
namespace sh::game
{
	PlayerPickup::PlayerPickup(GameObject& owner) :
		Component(owner)
	{
	}
	SH_USER_API void PlayerPickup::Awake()
	{
		player = gameObject.GetComponent<Player>();
		if (player == nullptr)
			SH_ERROR("Not found Player component!");
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