#include "Skill/ProjectileInstance.h"
#include "Skill/Projectile.h"

#include "Game/World.h"
namespace sh::game
{
	ProjectileInstance::ProjectileInstance(GameObject& owner) :
		Component(owner)
	{
	}

	SH_USER_API void ProjectileInstance::Awake()
	{
		if (rigidbody == nullptr)
			SH_ERROR("rigidbody is nullptr!");
	}

	SH_USER_API void ProjectileInstance::Update()
	{
		if (projectile == nullptr)
			return;

		t += world.deltaTime * 1000.f;
		if (t >= projectile->GetLifetime())
		{
			gameObject.Destroy();
		}
	}
	SH_USER_API void ProjectileInstance::Init(const Projectile& projectile, Entity* owner)
	{
		this->projectile = &projectile;
		if (core::IsValid(owner))
			this->owner = owner;
		rigidbody->SetLinearVelocity({ projectile.GetVelocity().x, projectile.GetVelocity().y, 0.f });
		rigidbody->ResetPhysicsTransform();
	}
	SH_USER_API auto ProjectileInstance::GetVelocity() const -> Vec2
	{
		if (rigidbody == nullptr)
			return { 0.f, 0.f };
		return { rigidbody->GetLinearVelocity().x, rigidbody->GetLinearVelocity().y };
	}
}//namespace