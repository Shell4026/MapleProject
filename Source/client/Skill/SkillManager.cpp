#include "Skill/SkillManager.h"
#include "Player/Player.h"
#include "Packet/SkillUsingPacket.hpp"
#include "MapleClient.h"

#include "Game/World.h"

// 클라측
namespace sh::game
{
	SH_USER_API void SkillManager::Awake()
	{
		if (player == nullptr)
			SH_ERROR("player is nullptr!");
	}
	SH_USER_API void SkillManager::BeginUpdate()
	{
		if (!player->IsLocal())
			return;

		UpdateState();

		if (Input::GetKeyDown(Input::KeyCode::A))
		{
			UseSkill(0);
		}
	}
	SH_USER_API void SkillManager::SetKeyBinding(Input::KeyCode keycode, SkillId skill)
	{
		keybindings[keycode] = skill;
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

		static MapleClient& client = *MapleClient::GetInstance();

		SkillUsingPacket packet{};
		packet.seq = seq++;
		packet.userUUID = client.GetUser().GetUserUUID();
		packet.skillId = id;
		client.SendPacket(packet);
	}
}//namespace