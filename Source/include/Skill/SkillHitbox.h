#pragma once
#include "Export.h"

#include "Game/Component/Component.h"
#include "Game/Component/Phys/RigidBody.h"
#include "Game/Vector.h"

namespace sh::game
{
	class Skill;
	class Player;
	class SkillHitbox : public Component
	{
		COMPONENT(SkillHitbox, "user")
	public:
		SH_USER_API SkillHitbox(GameObject& owner);

		SH_USER_API void Awake() override;
		SH_USER_API void BeginUpdate() override;

		SH_USER_API auto GetSkill() const -> Skill*;
		SH_USER_API auto GetPlayer() const-> Player*;
	private:
		PROPERTY(skill)
		Skill* skill = nullptr;
		PROPERTY(player)
		Player* player = nullptr;
		PROPERTY(rigidBody)
		RigidBody* rigidBody = nullptr;
		PROPERTY(collider)
		Collider* collider = nullptr;
	};
}
