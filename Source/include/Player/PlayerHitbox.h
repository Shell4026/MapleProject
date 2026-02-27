#pragma once
#include "../Export.h"

#include "Game/Component/Phys/BoxCollider.h"

namespace sh::game
{
	class Player;
	class PlayerHitbox : public BoxCollider
	{
		COMPONENT(PlayerHitbox, "user")
	public:
		SH_USER_API PlayerHitbox(GameObject& owner);

		SH_USER_API void Awake() override;

		SH_USER_API auto GetPlayer() const -> Player* { return player; }
	private:
		PROPERTY(player, core::PropertyOption::sobjPtr)
		Player* player = nullptr;
	};
}
