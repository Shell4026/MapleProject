#include "Mob/MobHitbox.h"
#include "Phys/CollisionTag.hpp"
namespace sh::game
{
	MobHitbox::MobHitbox(GameObject& owner) :
		BoxCollider(owner)
	{
		SetTrigger(true);
	}
	SH_USER_API void MobHitbox::Awake()
	{
		Super::Awake();
		SetCollisionTag(tag::mobHitboxTag);
		SetAllowCollisions(tag::projectileHitboxTag);
	}
}//namespace