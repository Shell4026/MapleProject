#pragma once
#include "Export.h"
#include "Mob.h"

#include "Game/Component/Component.h"

namespace sh::game
{
	class RigidBody;
	class MobHitbox : public Component
	{
		COMPONENT(MobHitbox, "user")
	public:
		SH_USER_API MobHitbox(GameObject& owner);

		SH_USER_API void Awake() override;
		SH_USER_API void BeginUpdate() override;
		SH_USER_API void OnTriggerEnter(Collider& other) override;
	private:
		PROPERTY(mob)
		Mob* mob = nullptr;
		PROPERTY(rigidbody)
		RigidBody* rigidbody = nullptr;
		PROPERTY(collider)
		Collider* collider = nullptr;
	};
}
