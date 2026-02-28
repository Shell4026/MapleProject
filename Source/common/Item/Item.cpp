#include "Item/Item.h"
#include "World/MapleWorld.h"
#include "Phys/CollisionTag.hpp"
#include "Phys/FootholdMovement.h"

#include "Game/GameObject.h"

namespace sh::game
{
	SH_USER_API void Item::OnDisable()
	{
		if (movement != nullptr)
			movement->SetVelocity(0.f, 0.f);
	}
	SH_USER_API void Item::FixedUpdate()
	{
		if (movement != nullptr)
			movement->StepMovement();
	}
	SH_USER_API void Item::Update()
	{
		if (rb != nullptr && movement != nullptr)
		{
			Vec2 vel = movement->GetVelocity();
			if (std::abs(vel.x) > 0.f || std::abs(vel.y) > 0.f)
				rb->ResetPhysicsTransform();
		}
	}
	SH_USER_API void Item::SetCurrentWorld(MapleWorld& world)
	{
		mapleWorld = &world;
		if (movement != nullptr)
			movement->SetFoothold(*world.GetFoothold());
	}
}//namespace