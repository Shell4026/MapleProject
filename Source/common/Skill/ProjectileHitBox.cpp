#include "Skill/ProjectileHitBox.h"
#include "Phys/CollisionTag.hpp"
namespace sh::game
{
	ProjectileHitBox::ProjectileHitBox(GameObject& owner) :
		BoxCollider(owner)
	{
	}
	SH_USER_API void ProjectileHitBox::Awake()
	{
		Super::Awake();
		SetTrigger(true);
		SetCollisionTag(tag::projectileHitboxTag);
		SetAllowCollisions(tag::mobHitboxTag);
	}
}//namespace