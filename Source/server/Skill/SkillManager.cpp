#include "Skill/SkillManager.h"
#include "World/MapleServer.h"
#include "Player/Player.h"

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
		while (!pendingSkills.empty() && pendingSkills.front().applyServerTick <= tick)
		{
			currentSkill = pendingSkills.front();
			pendingSkills.pop_front();
		}

		UseSkill(currentSkill.skillId, tick);
		UpdateState();
	}
	SH_USER_API void SkillManager::ProcessPacket(const SkillUsingPacket& packet)
	{
		if (lastSeq >= packet.seq)
			return;
		lastSeq = packet.seq;

		const uint64_t tick = player != nullptr ? player->GetTick() : 0;
		if (!bOffsetInit)
		{
			offset = tick - packet.tick + 5;
			bOffsetInit = true;
		}
		else
		{
			const uint64_t newOffset = tick - packet.tick + 5;
			offset = (offset * 9 + newOffset) / 10;
		}

		PendingSkill pending{};
		pending.seq = packet.seq;
		pending.skillId = packet.skillId;
		pending.applyServerTick = packet.tick + offset;
		pendingSkills.push_back(pending);
	}
	void SkillManager::UseSkill(SkillId id, uint64_t tick)
	{
		if (id == 0 || !CanUse(id))
			return;

		SkillState* const state = GetSkillState(id);
		if (state == nullptr)
			return;

		state->state = SkillState::State::Start;
		ApplyCooldown(id);
		lastState = state;
		SH_INFO_FORMAT("Use skill: {}", id);
	}
}//namespace
