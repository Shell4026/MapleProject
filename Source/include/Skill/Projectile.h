#pragma once
#include "Export.h"
#include "Entity.h"

#include "Game/Vector.h"
#include "Game/ScriptableObject.h"
#include "Game/Prefab.h"
namespace sh::game
{
	class Projectile : public ScriptableObject
	{
		SRPO(Projectile)
	public:
		SH_USER_API void SpawnProjectile(World& world, Entity* owner, float x, float y, bool bRight);

		SH_USER_API auto GetDamage() const -> int { return damage; }
		SH_USER_API auto GetVelocity() const -> Vec2 { return velocity; }
		SH_USER_API auto GetLifetime() const -> uint32_t { return lifetimeMs; }
		SH_USER_API auto GetPrefab() const -> Prefab* { return prefab; }
	private:
		PROPERTY(damage)
		int damage = 0;
		PROPERTY(velocity)
		Vec2 velocity{ 0.f, 0.f };
		PROPERTY(lifetimeMs)
		uint32_t lifetimeMs = 0;
		PROPERTY(prefab)
		Prefab* prefab = nullptr;
	};
}//namespace