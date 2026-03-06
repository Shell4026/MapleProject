#include "Skill/SkillManager.h"
#include "Skill/Buff.h"
#include "Skill/Projectile.h"
#include "Skill/ProjectileInstance.h"
#include "Player/Player.h"
#include "Player/PlayerMovement.h"

#include "Core/Logger.h"

#include "Game/World.h"
#include <algorithm>
// 공용
namespace sh::game
{
	SkillManager::SkillManager(GameObject& owner) :
		Component(owner)
	{
	}
	SH_USER_API void SkillManager::RegisterSkill(Skill& skill)
	{
		auto it = skillStateIdxs.find(skill.GetId());
		if (it != skillStateIdxs.end())
			return;
		skillStateIdxs.insert({ skill.GetId(), skillStates.size() });
		auto& state = skillStates.emplace_back();
		state.skill = &skill;
		state.skillId = skill.GetId();

		SH_INFO_FORMAT("Register skill: {}", skill.GetId());
	}
	SH_USER_API void SkillManager::UnRegisterSkill(SkillId id)
	{
		auto it = skillStateIdxs.find(id);
		if (it == skillStateIdxs.end())
			return;
		const std::size_t idx = it->second;
		skillStateIdxs.erase(id);
	}
	SH_USER_API auto SkillManager::CanUse(SkillId id) const -> bool
	{
		const SkillState* const state = GetSkillState(id);
		if (state == nullptr)
			return false;

		if (lastState != nullptr && !state->skill->IsAllowSkill(lastState->skillId))
			return false;

		if (!(state->cooldownRemainingMs == 0.f && state->state == SkillState::State::Wait))
			return false;

		for (const SkillCondition* condition : state->skill->GetConditions())
		{
			if (!core::IsValid(condition))
				continue;
			if (!CheckCondition(*condition, *state))
				return false;
		}
		return true;
	}
	SH_USER_API void SkillManager::ApplyCooldown(SkillId id)
	{
		const Skill* const skillPtr = GetSkill(id);
		if (skillPtr == nullptr)
			return;

		SkillState* const state = GetSkillState(id);
		if (state == nullptr)
			return;

		state->cooldownRemainingMs = static_cast<float>(skillPtr->GetCooldownMs());
	}
	SH_USER_API auto SkillManager::IsOnCooldown(SkillId id) const -> bool
	{
		const SkillState* const state = GetSkillState(id);
		if (state == nullptr)
			return false;
		return state->cooldownRemainingMs > 0.f;
	}
	SH_USER_API auto SkillManager::GetSkill(SkillId id) const -> Skill*
	{
		auto it = skillStateIdxs.find(id);
		if (it == skillStateIdxs.end())
			return nullptr;
		return skillStates[it->second].skill;
	}
	SH_USER_API auto SkillManager::GetSkillState(SkillId id) const -> const SkillState*
	{
		auto it = skillStateIdxs.find(id);
		if (it == skillStateIdxs.end())
			return nullptr;
		return &skillStates[it->second];
	}
	auto SkillManager::GetSkillState(SkillId id) -> SkillState*
	{
		auto it = skillStateIdxs.find(id);
		if (it == skillStateIdxs.end())
			return nullptr;
		return &skillStates[it->second];
	}
	void SkillManager::UpdateConditionState(uint64_t tick)
	{
		if (!core::IsValid(player))
			return;

		PlayerMovement* const movement = player->GetMovement();
		if (!core::IsValid(movement))
			return;

		if (movement->IsLandedThisTick())
			lastLandedTick = tick - 1; // SkillManager -> Movement순서로 실행 되기 때문에 이전틱임
		if (movement->IsJumpTriggered())
			lastJumpTick = tick - 1;
	}
	auto SkillManager::CheckCondition(const SkillCondition& condition, const SkillState& state) const -> bool
	{
		switch (condition.GetConditionType())
		{
		case SkillCondition::Type::None:
			return true;
		case SkillCondition::Type::LandedAfterLastUse:
			return lastLandedTick > state.lastUsedTick;
		case SkillCondition::Type::JumpAfterLastUse:
			return lastJumpTick > state.lastUsedTick;
		case SkillCondition::Type::PreviousSkillIn:
		{
			const auto& requiredSkills = condition.GetRequiredSkills();
			return std::find(requiredSkills.begin(), requiredSkills.end(), lastUsedSkillId) != requiredSkills.end();
		}
		default:
			return true;
		}
	}
	void SkillManager::UpdateCooldown(SkillState& state)
	{
		const float dt = world.FIXED_TIME * 1000.f;
		float& cooldown = state.cooldownRemainingMs;
		if (cooldown <= 0.f)
			return;

		cooldown -= dt;
		if (cooldown < 0.f)
			cooldown = 0.f;
	}
	void SkillManager::UpdateState()
	{
		const float dt = world.FIXED_TIME * 1000.f;

		for (std::size_t i = 0; i < skillStates.size(); ++i)
		{
			SkillState& state = skillStates[i];
			Skill* const skillPtr = state.skill;
			if (!core::IsValid(skillPtr))
			{
				RemoveInvalidState(i);
				--i;
				continue;
			}

			UpdateCooldown(state);
			if (state.state == SkillState::State::Wait)
				continue;

			if (skillPtr->IsPreventMove())
				player->GetMovement()->LockInput();

			const SkillState::State prevState = state.state;
			const bool bEnterStart = prevState == SkillState::State::Start && state.counterMs == 0.f;
			const uint32_t startupMs = skillPtr->GetStartupMs();
			const uint32_t activeEndMs = startupMs + skillPtr->GetActiveMs();
			const uint32_t skillEndMs = activeEndMs + skillPtr->GetRecoveryMs();

			state.counterMs += dt;
			if (state.counterMs > skillEndMs)
			{
				if (lastState == &state)
					lastState = nullptr;

				state.state = SkillState::State::Wait;
				state.counterMs = 0.f;

				if (skillPtr->IsPreventMove())
					player->GetMovement()->UnlockInput();
			}
			else if (state.counterMs > activeEndMs)
				state.state = SkillState::State::Recovery;
			else if (state.counterMs > startupMs)
				state.state = SkillState::State::Active;

			if (bEnterStart)
			{
				ApplySkillBuffs(*skillPtr, SkillState::State::Start);
				if (!skillPtr->IsAllowContinousInput() && pressedSkillId == state.skillId)
					pressedSkillId = 0;
			}

			if (state.state == prevState)
				continue;

			ApplySkillBuffs(*skillPtr, state.state);
			if (state.state == SkillState::State::Active)
			{
				const auto& pos = gameObject.transform->GetWorldPosition();
				for (auto projectile : skillPtr->GetProjectiles())
				{
					if (projectile == nullptr)
						continue;

					projectile->SpawnProjectile(world, player, pos.x, pos.y, player->GetMovement()->IsRight());
				}
			}
		}
	}
	void SkillManager::RemoveInvalidState(std::size_t idx)
	{
		SkillState& state = skillStates[idx];
		skillStateIdxs.erase(state.skillId);

		const std::size_t lastIdx = skillStates.size() - 1;
		if (idx != lastIdx)
		{
			skillStates[idx] = skillStates.back();
			skillStateIdxs[skillStates[idx].skillId] = idx;
		}

		skillStates.pop_back();
	}
	void SkillManager::ApplySkillBuffs(Skill& skill, SkillState::State phase)
	{
		if (!core::IsValid(player))
			return;

		for (Buff* buff : skill.GetBuffs())
		{
			if (!core::IsValid(buff))
				continue;

			const Buff::ApplyPhase applyPhase = buff->GetApplyPhase();
			const SkillState::State targetPhase =
				applyPhase == Buff::ApplyPhase::Start ? SkillState::State::Start :
				(applyPhase == Buff::ApplyPhase::Active ? SkillState::State::Active : SkillState::State::Recovery);
			if (targetPhase != phase)
				continue;

			buff->OnApply(*player, skill);
		}
	}
}//namespace
