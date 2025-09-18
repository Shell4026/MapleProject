﻿#include "MobHitbox.h"
#include "CollisionTag.hpp"
#include "SkillHitbox.h"
#include "Mob.h"
#include "Player.h"
#include "Skill.h"

#include "Game/GameObject.h"
#include "Game/Component/Collider.h"
#include "Game/Component/RigidBody.h"
namespace sh::game
{
	MobHitbox::MobHitbox(GameObject& owner) :
		Component(owner)
	{
	}
	SH_USER_API void MobHitbox::Awake()
	{
		if (collider != nullptr)
		{
			collider->SetCollisionTag(tag::mobHitboxTag);
			collider->SetAllowCollisions(tag::skillHitboxTag);
		}
	}
	SH_USER_API void MobHitbox::BeginUpdate()
	{
		if (rigidbody != nullptr)
			rigidbody->ResetPhysicsTransform();
	}
	SH_USER_API void MobHitbox::OnTriggerEnter(Collider& other)
	{
#if SH_SERVER
		if (!core::IsValid(mob) || collider == nullptr)
			return;

		GameObject& obj = other.gameObject;
		auto skillHitboxPtr = obj.GetComponent<SkillHitbox>();
		if (!core::IsValid(skillHitboxPtr))
			return;

		Player* playerPtr = skillHitboxPtr->GetPlayer();
		Skill* skillPtr = skillHitboxPtr->GetSkill();
		if (core::IsValid(playerPtr) && core::IsValid(skillPtr))
			mob->Hit(*skillPtr, *playerPtr);
#endif
	}
}//namespace