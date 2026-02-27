#include "Skill/SkillManager.h"
#include "MapleServer.h"

// 서버측
namespace sh::game
{
	SH_USER_API void SkillManager::Awake()
	{
	}
	SH_USER_API void SkillManager::BeginUpdate()
	{
		UpdateState();
	}
	SH_USER_API void SkillManager::ProcessPacket(const SkillUsingPacket& packet)
	{
		if (lastSeq > packet.seq)
			return;
		lastSeq = packet.seq;

		UseSkill(packet.skillId);
	}
	void SkillManager::UseSkill(SkillId id)
	{
		if (!CanUse(id))
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