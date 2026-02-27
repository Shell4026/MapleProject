#include "Player/PlayerHitbox.h"
#include "CollisionTag.hpp"
namespace sh::game
{
	PlayerHitbox::PlayerHitbox(GameObject& owner) :
		BoxCollider(owner)
	{
		SetTrigger(true);
	}
	SH_USER_API void PlayerHitbox::Awake()
	{
		Super::Awake();
		SetCollisionTag(tag::playerTag);
		SetAllowCollisions(tag::itemTag);
	}
}//namespace