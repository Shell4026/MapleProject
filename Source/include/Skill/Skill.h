#pragma once
#include "Export.h"

#include "Game/ScriptableObject.h"

#include <vector>
namespace sh::game
{
	class Projectile;
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
		SH_USER_API auto IsPreventMove() const -> bool { return bPreventMove; }
		SH_USER_API auto IsPreventAir() const -> bool { return bPreventAir; }
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

		PROPERTY(projectiles, core::PropertyOption::sobjPtr)
		std::vector<Projectile*> projectiles;

		PROPERTY(bPreventMove)
		bool bPreventMove = true;
		PROPERTY(bPreventAir)
		bool bPreventAir = false;
	};
}//namespace