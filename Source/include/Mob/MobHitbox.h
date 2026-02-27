#pragma once
#include "../Export.h"

#include "Game/Component/Phys/BoxCollider.h"

namespace sh::game
{
	class Mob;
	class MobHitbox : public BoxCollider
	{
		COMPONENT(MobHitbox, "user")
	public:
		SH_USER_API MobHitbox(GameObject& owner);

		SH_USER_API void Awake() override;
	private:
		PROPERTY(mob, core::PropertyOption::sobjPtr)
		Mob* mob = nullptr;
	};
}
