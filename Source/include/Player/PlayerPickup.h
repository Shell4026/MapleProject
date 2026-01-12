#pragma once
#include "Export.h"
#include "PacketEvent.hpp"
#include "Player.h"
#include "Item/Item.h"
#include "Database.h"

#include "Core/EventSubscriber.h"

#include "Game/Component/Component.h"

#include <Core/SContainer.hpp>

#include <cstdint>
namespace sh::game
{
	class PlayerPickup : public Component
	{
		COMPONENT(PlayerPickup, "user")
	public:
		SH_USER_API PlayerPickup(GameObject& owner);

		SH_USER_API void Awake() override;
		SH_USER_API void BeginUpdate() override;
		SH_USER_API void Update() override;
		SH_USER_API void OnTriggerEnter(Collider& collider) override;
		SH_USER_API void OnTriggerExit(Collider& collider) override;
	private:
#if SH_SERVER
		void ProcessPickup();
		void InsertItemToInventory(Item& item, Player& player);
#endif
	private:
		core::EventSubscriber<PacketEvent> packetSubscriber;
		Player* player = nullptr;
#if SH_SERVER
		core::SMap<uint64_t, Item*> hitItems;
		
		bool bPickupState = false;
#endif
	};
}//namespace