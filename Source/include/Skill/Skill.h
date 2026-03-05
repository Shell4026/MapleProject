#pragma once
#include "Export.h"
#include "Skill/SkillCondition.h"

#include "Game/ScriptableObject.h"

#include <vector>
namespace sh::game
{
	class Projectile;
	class Buff;
	class Skill : public ScriptableObject
	{
		SRPO(Skill)
	public:
		SH_USER_API auto GetId() const -> uint32_t { return id; }
		SH_USER_API auto GetStartupMs() const -> uint32_t { return startupMs; }
		SH_USER_API auto GetActiveMs() const -> uint32_t { return activeMs; }
		SH_USER_API auto GetRecoveryMs() const -> uint32_t { return recoveryMs; }
		SH_USER_API auto GetCooldownMs() const -> uint32_t { return cooldownMs; }
		SH_USER_API auto GetAnimState() const -> uint32_t { return animState; }
		SH_USER_API auto GetProjectiles() const -> const std::vector<Projectile*>& { return projectiles; }
		SH_USER_API auto GetEffects() const -> const std::vector<Projectile*>& { return effects; }
		SH_USER_API auto GetBuffs() const -> const std::vector<Buff*>& { return buffs; }
		SH_USER_API auto GetConditions() const -> const std::vector<SkillCondition*>& { return conditions; }
		SH_USER_API auto IsPreventMove() const -> bool { return bPreventMove; }
		SH_USER_API auto IsPreventAir() const -> bool { return bPreventAir; }
		SH_USER_API auto IsAllowSkill(uint32_t skillId) -> bool { return std::find(allowSkill.begin(), allowSkill.end(), skillId) != allowSkill.end(); }
		SH_USER_API auto IsAllowContinousInput() const -> bool { return bContinousInput; }
	private:
		PROPERTY(id)
		uint32_t id = 0;
		PROPERTY(startupMs)
		uint32_t startupMs = 0; // 선딜
		PROPERTY(activeMs)
		uint32_t activeMs = 0; // 판정 구간
		PROPERTY(recoveryMs)
		uint32_t recoveryMs = 0; // 후딜
		PROPERTY(cooldownMs)
		uint32_t cooldownMs = 0;
		PROPERTY(animState)
		uint32_t animState = 0;

		PROPERTY(allowSkill)
		std::vector<uint32_t> allowSkill; // 동시에 쓸 수 있는 스킬 리스트
		PROPERTY(projectiles, core::PropertyOption::sobjPtr)
		std::vector<Projectile*> projectiles;
		PROPERTY(effects, core::PropertyOption::sobjPtr)
		std::vector<Projectile*> effects;
		PROPERTY(buffs, core::PropertyOption::sobjPtr)
		std::vector<Buff*> buffs;
		PROPERTY(conditions, core::PropertyOption::sobjPtr)
		std::vector<SkillCondition*> conditions;

		PROPERTY(bPreventMove)
		bool bPreventMove = true;
		PROPERTY(bPreventAir)
		bool bPreventAir = false;
		PROPERTY(bContinousInput)
		bool bContinousInput = true;
	};
}//namespace
