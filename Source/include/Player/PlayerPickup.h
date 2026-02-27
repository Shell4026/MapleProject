#pragma once
#include "Export.h"
#include "Item/Item.h"

#include "Game/Component/Component.h"

#include "Network/PacketEvent.hpp"

#include <cstdint>
namespace sh::game
{
	class Player;
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

#if SH_SERVER
		SH_USER_API void SetPickupState(bool bState) { bPickupState = bState; }
		SH_USER_API auto IsPickupState() const -> bool { return bPickupState; }
#endif
	private:
#if SH_SERVER
		void ProcessPickup();
		void InsertItemToInventory(Item& item, Player& player);
#endif
	private:
		PROPERTY(player, core::PropertyOption::sobjPtr)
		Player* player = nullptr;
#if SH_SERVER
		core::SMap<uint64_t, Item*> hitItems;
		
		bool bPickupState = false;
#endif
	};
}//namespace