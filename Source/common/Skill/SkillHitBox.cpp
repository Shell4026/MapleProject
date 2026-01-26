#include "Skill/SkillHitbox.h"
#include "CollisionTag.hpp"

#include "Game/Component/Phys/Collider.h"
#include "Game/Component/Phys/RigidBody.h"
namespace sh::game
{
	SkillHitbox::SkillHitbox(GameObject& owner) :
		Component(owner)
	{
	}
	SH_USER_API void SkillHitbox::Awake()
	{
		if (collider != nullptr)
		{
			collider->SetCollisionTag(tag::skillHitboxTag);
			collider->SetAllowCollisions(tag::mobHitboxTag);
		}
	}
	SH_USER_API void SkillHitbox::BeginUpdate()
	{
		if (core::IsValid(rigidBody))
			rigidBody->ResetPhysicsTransform();
	}
	SH_USER_API auto SkillHitbox::GetSkill() const -> Skill*
	{
		return skill;
	}
	SH_USER_API auto SkillHitbox::GetPlayer() const -> Player*
	{
		return player;
	}
}//namespace