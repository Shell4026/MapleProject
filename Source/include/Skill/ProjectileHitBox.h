#pragma once
#include "Export.h"

#include "Game/Component/Component.h"
#include "Game/Component/Phys/BoxCollider.h"

namespace sh::game
{
	class ProjectileInstance;
	class ProjectileHitBox : public BoxCollider
	{
		COMPONENT(ProjectileHitBox, "user")
	public:
		SH_USER_API ProjectileHitBox(GameObject& owner);

		SH_USER_API void Awake() override;

		SH_USER_API auto GetProjectileInstance() const -> ProjectileInstance* { return projectileInstance; }
	private:
		PROPERTY(projectileInstance, core::PropertyOption::sobjPtr)
		ProjectileInstance* projectileInstance = nullptr;
	};
}//namespace