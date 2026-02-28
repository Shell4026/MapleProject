#include "Skill/SkillManager.h"
#include "Player/Player.h"
#include "Packet/SkillUsingPacket.hpp"
#include "World/MapleClient.h"

#include "Game/World.h"

// 클라측
namespace sh::game
{
	SH_USER_API void SkillManager::Awake()
	{
		if (player == nullptr)
			SH_ERROR("player is nullptr!");
	}
	SH_USER_API void SkillManager::TickBegin(uint64_t tick)
	{
		if (!player->IsLocal())
			return;

		SkillId skillId = 0;
		if (Input::GetKeyDown(Input::KeyCode::A))
		{
			skillId = 1;
		}

		const bool bChanged = lastInput.skillId != skillId;
		if (bChanged)
		{
			static MapleClient& client = *MapleClient::GetInstance();

			SkillUsingPacket packet{};
			packet.seq = seq++;
			packet.tick = tick;
			packet.userUUID = client.GetUser().GetUserUUID();
			packet.skillId = skillId;
			client.SendPacket(packet);
		}

		lastInput.skillId = skillId;
	}
	SH_USER_API void SkillManager::TickFixed(uint64_t tick)
	{
		if (!player->IsLocal())
			return;

		if (lastInput.skillId != 0)
		{
			UseSkill(lastInput.skillId, tick);
		}

		UpdateState();
	}
	SH_USER_API void SkillManager::SetKeyBinding(Input::KeyCode keycode, SkillId skill)
	{
		keybindings[keycode] = skill;
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
