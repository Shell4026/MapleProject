#pragma once
#include "../Export.h"

#include "Game/Component/Phys/BoxCollider.h"

namespace sh::game
{
	class Item;
	class ItemHitbox : public BoxCollider
	{
		COMPONENT(ItemHitbox, "user")
	public:
		SH_USER_API ItemHitbox(GameObject& owner);

		SH_USER_API void Awake() override;

		SH_USER_API auto GetItem() const -> Item* { return item; }
	private:
		PROPERTY(item, core::PropertyOption::sobjPtr)
		Item* item = nullptr;
	};
}
