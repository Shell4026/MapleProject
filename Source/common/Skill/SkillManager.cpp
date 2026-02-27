#include "Skill/SkillManager.h"
#include "Skill/Projectile.h"
#include "Skill/ProjectileInstance.h"
#include "Player/Player.h"
#include "Player/PlayerMovement.h"

#include "Core/Logger.h"

#include "Game/World.h"
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
		return state->cooldownRemainingMs == 0.f && state->state == SkillState::State::Wait;
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
	void SkillManager::UpdateState()
	{
		for (int i = 0; i < skillStates.size(); ++i)
		{
			SkillState& state = skillStates[i];
			Skill* const skillPtr = state.skill;
			if (core::IsValid(skillPtr))
			{
				float& cooldown = state.cooldownRemainingMs;
				if (cooldown > 0.f)
				{
					cooldown -= world.deltaTime * 1000.f;
					if (cooldown < 0.f)
						cooldown = 0.f;
				}
				if (state.state != SkillState::State::Wait)
				{
					if (state.skill->IsPreventMove())
						player->GetMovement()->LockInput();

					auto prevState = state.state;
					const uint32_t skillTime = skillPtr->GetStartupMs() + skillPtr->GetActiveMs() + skillPtr->GetRecoveryMs();
					state.counterMs += world.deltaTime * 1000.f;
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

					if (state.state != prevState)
					{
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
}//namespace