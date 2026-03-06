#include "Skill/SkillCondition.h"

#include <algorithm>

namespace sh::game
{
	SH_USER_API auto LandedAfterLastUseSkillCondition::Evaluate(const SkillConditionContext& context) const -> bool
	{
		return context.lastLandedTick > context.lastUsedTick;
	}

	SH_USER_API auto JumpAfterLastUseSkillCondition::Evaluate(const SkillConditionContext& context) const -> bool
	{
		return context.lastJumpTick > context.lastUsedTick;
	}

	SH_USER_API auto PreviousSkillInSkillCondition::Evaluate(const SkillConditionContext& context) const -> bool
	{
		return std::find(requiredSkills.begin(), requiredSkills.end(), context.lastUsedSkillId) != requiredSkills.end();
	}
}//namespace
