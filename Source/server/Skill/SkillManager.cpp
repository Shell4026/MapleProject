#include "Skill/SkillManager.h"
#include "World/MapleServer.h"
#include "Player/Player.h"
#include "Player/PlayerMovement.h"

// 서버측
namespace sh::game
{
	SH_USER_API void SkillManager::Awake()
	{
	}
	SH_USER_API void SkillManager::TickBegin(uint64_t tick)
	{
	}
	SH_USER_API void SkillManager::TickFixed(uint64_t tick)
	{
		UpdateConditionState(tick);

		while (!pendingSkills.empty() && pendingSkills.front().applyServerTick <= tick)
		{
			const PendingSkill pending = pendingSkills.front();
			pendingSkills.pop_front();

			if (pending.action == SkillInputAction::Pressed)
			{
				if (pending.dir != 0 && player != nullptr && player->GetMovement() != nullptr)
					player->GetMovement()->SetFacing(pending.dir > 0);
				pressedSkillId = pending.skillId;
			}
			else if (pending.action == SkillInputAction::Released)
			{
				if (pressedSkillId == pending.skillId)
					pressedSkillId = 0;
			}
		}

		UseSkill(pressedSkillId, tick);
		UpdateState();
	}
	SH_USER_API void SkillManager::ProcessPacket(const SkillUsingPacket& packet)
	{
		if (!pendingSkills.empty() && pendingSkills.back().seq >= packet.seq)
			return;
		//SH_INFO_FORMAT("recv seq: {}, tick: {}, serverTick: {}, offset: {}", packet.seq, packet.tick, tick, offset);

		PendingSkill pending{};
		pending.seq = packet.seq;
		pending.skillId = packet.skillId;
		pending.action = packet.action;
		pending.dir = packet.dir;
		pending.applyServerTick = player->EstimateApplyServerTick(packet.tick);
		pendingSkills.push_back(pending);
	}
	void SkillManager::UseSkill(SkillId id, uint64_t tick)
	{
		SkillState* const state = GetSkillState(id);
		if (state == nullptr)
			return;

		if (id == 0 || !CanUse(id))
		{
			if (!state->skill->IsAllowContinousInput())
				pressedSkillId = 0;
			return;
		}

		state->state = SkillState::State::Start;
		state->lastUsedTick = tick;

		ApplyCooldown(id);

		lastState = state;
		lastUsedSkillId = id;
		SH_INFO_FORMAT("Use skill: {}", id);
	}
}//namespace
