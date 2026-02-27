#pragma once
#include "Export.h"

#include "Game/Component/Component.h"
#include "Game/Component/Phys/RigidBody.h"
#include "Game/Vector.h"

#include <cstdint>
namespace sh::game
{
	class Entity;
	class Projectile;
	class ProjectileInstance : public Component
	{
		COMPONENT(ProjectileInstance, "user")
	public:
		SH_USER_API ProjectileInstance(GameObject& owner);

		SH_USER_API void Awake() override;
		SH_USER_API void Update() override;

		SH_USER_API void Init(const Projectile& projectile, Entity* owner);

		SH_USER_API auto GetOwner() const -> Entity* { return owner; }
		SH_USER_API auto GetVelocity() const -> Vec2;
		SH_USER_API auto GetRigidbody() const -> RigidBody* { return rigidbody; }
		SH_USER_API auto GetProjectile() const -> const Projectile* { return projectile; }
	private:
		PROPERTY(rigidbody)
		RigidBody* rigidbody = nullptr;
		PROPERTY(projectile, core::PropertyOption::sobjPtr, core::PropertyOption::invisible)
		const Projectile* projectile = nullptr;
		PROPERTY(owner, core::PropertyOption::sobjPtr, core::PropertyOption::invisible)
		Entity* owner = nullptr;
		float t = 0.f;
	};
}//namespace