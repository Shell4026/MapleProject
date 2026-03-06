#pragma once
#include "Export.h"

#include "Game/ScriptableObject.h"

#include <cstdint>
#include <vector>
namespace sh::game
{
	struct SkillConditionContext
	{
		uint64_t lastLandedTick = 0;
		uint64_t lastJumpTick = 0;
		uint64_t lastUsedTick = 0;
		uint32_t lastUsedSkillId = 0;
	};

	class SkillCondition : public ScriptableObject
	{
		SCLASS(SkillCondition)
	public:
		SH_USER_API virtual auto Evaluate(const SkillConditionContext& context) const -> bool = 0;
	};

	class LandedAfterLastUseSkillCondition : public SkillCondition
	{
		SRPO(LandedAfterLastUseSkillCondition)
	public:
		SH_USER_API auto Evaluate(const SkillConditionContext& context) const -> bool override;
	};

	class JumpAfterLastUseSkillCondition : public SkillCondition
	{
		SRPO(JumpAfterLastUseSkillCondition)
	public:
		SH_USER_API auto Evaluate(const SkillConditionContext& context) const -> bool override;
	};

	class PreviousSkillInSkillCondition : public SkillCondition
	{
		SRPO(PreviousSkillInSkillCondition)
	public:
		SH_USER_API auto Evaluate(const SkillConditionContext& context) const -> bool override;

		SH_USER_API auto GetRequiredSkills() const -> const std::vector<uint32_t>& { return requiredSkills; }
	private:
		PROPERTY(requiredSkills)
		std::vector<uint32_t> requiredSkills;
	};
}//namespace
