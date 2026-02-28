#include "Item/ItemHitbox.h"
#include "Phys/CollisionTag.hpp"
namespace sh::game
{
	ItemHitbox::ItemHitbox(GameObject& owner) :
		BoxCollider(owner)
	{
		SetTrigger(true);
	}
	SH_USER_API void ItemHitbox::Awake()
	{
		Super::Awake();
		SetCollisionTag(tag::itemTag);
		SetAllowCollisions(tag::playerTag);
	}
}//namespace