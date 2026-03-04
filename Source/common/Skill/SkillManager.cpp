#include "Skill/SkillManager.h"
#include "Skill/MovementSkill.h"
#include "Skill/Projectile.h"
#include "Skill/ProjectileInstance.h"
#include "Player/Player.h"
#include "Player/PlayerMovement.h"

#include "Core/Logger.h"

#include "Game/World.h"
#include <algorithm>
#include <cmath>
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
			lastLandedTick = tick;
		if (movement->IsJumpTriggered())
			lastJumpTick = tick;
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
	void SkillManager::UpdateState()
	{
		const float dt = world.FIXED_TIME * 1000.f;
		for (int i = 0; i < skillStates.size(); ++i)
		{
			SkillState& state = skillStates[i];
			Skill* const skillPtr = state.skill;
			if (core::IsValid(skillPtr))
			{
				float& cooldown = state.cooldownRemainingMs;
				if (cooldown > 0.f)
				{
					cooldown -= dt;
					if (cooldown < 0.f)
						cooldown = 0.f;
				}
				if (state.state != SkillState::State::Wait)
				{
					if (state.skill->IsPreventMove())
						player->GetMovement()->LockInput();

					auto prevState = state.state;
					const bool bStartEntered = state.state == SkillState::State::Start && state.counterMs == 0.f;
					const uint32_t skillTime = skillPtr->GetStartupMs() + skillPtr->GetActiveMs() + skillPtr->GetRecoveryMs();
					state.counterMs += dt;
					if (state.counterMs > skillTime)
					{
						if (lastState == &state)
							lastState = nullptr;
						state.state = SkillState::State::Wait;
						state.counterMs = 0.f;
						if (state.skill->IsPreventMove())
							player->GetMovement()->UnlockInput();
					}
					else if (state.counterMs > skillPtr->GetStartupMs() + skillPtr->GetActiveMs())
						state.state = SkillState::State::Recovery;
					else if (state.counterMs > skillPtr->GetStartupMs())
						state.state = SkillState::State::Active;

					if (bStartEntered)
					{
						ApplyMovementSkill(skillPtr, SkillState::State::Start);
						if (!state.skill->IsAllowContinousInput())
						{
							if (pressedSkillId == state.skillId)
								pressedSkillId = 0;
						}
					}

					if (state.state != prevState)
					{
						ApplyMovementSkill(skillPtr, state.state);

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
			}
			else
			{
				if (i != skillStates.size() - 1)
				{
					skillStateIdxs.erase(state.skillId);
					skillStateIdxs[skillStates.back().skill->GetId()] = i;
					skillStates[i] = skillStates.back();
					skillStates.pop_back();
					--i;
					continue;
				}
				else
				{
					skillStateIdxs.erase(state.skillId);
					skillStates.pop_back();
				}
			}
		}
	}
	void SkillManager::ApplyMovementSkill(Skill* skill, SkillState::State phase)
	{
		MovementSkill* const movementSkill = core::reflection::Cast<MovementSkill>(skill);
		if (movementSkill == nullptr)
			return;

		PlayerMovement* const movement = player->GetMovement();
		if (!core::IsValid(movement))
			return;

		if (movementSkill->IsRequireGround() && !movement->IsGround())
			return;

		const MovementSkill::ApplyPhase applyPhase = movementSkill->GetApplyPhase();
		const SkillState::State targetPhase =
			applyPhase == MovementSkill::ApplyPhase::Start ? SkillState::State::Start :
			(applyPhase == MovementSkill::ApplyPhase::Active ? SkillState::State::Active : SkillState::State::Recovery);
		if (targetPhase != phase)
			return;

		float moveX = movementSkill->GetMoveX();
		float moveY = movementSkill->GetMoveY();
		if (movementSkill->IsUseFacing() && !movement->IsRight())
			moveX = -moveX;

		//const float maxDistance = movementSkill->GetMaxDistance();
		//if (maxDistance > 0.f)
		//{
		//	const float lenSq = moveX * moveX + moveY * moveY;
		//	if (lenSq > maxDistance * maxDistance)
		//	{
		//		const float len = std::sqrt(lenSq);
		//		if (len > 0.f)
		//		{
		//			const float ratio = maxDistance / len;
		//			moveX *= ratio;
		//			moveY *= ratio;
		//		}
		//	}
		//}

		if (movementSkill->GetMoveType() == MovementSkill::MoveType::Impulse)
		{
			movement->ApplySkillImpulse(moveX, moveY);
		}
		else
		{
			movement->ApplySkillTeleport(moveX, moveY);
		}
	}
}//namespace
