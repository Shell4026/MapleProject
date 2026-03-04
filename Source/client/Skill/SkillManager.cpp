#include "Skill/SkillManager.h"
#include "Skill/Projectile.h"
#include "Player/Player.h"
#include "Player/PlayerMovement.h"
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

		if (Input::GetKeyPressed(Input::KeyCode::A))
		{
			SendPacket(1, SkillInputAction::Pressed, tick);
			pressedSkillId = 1;
		}
		else if (Input::GetKeyReleased(Input::KeyCode::A))
		{
			SendPacket(1, SkillInputAction::Released, tick);
			if (pressedSkillId == 1)
				pressedSkillId = 0;
		}

		if (Input::GetKeyPressed(Input::KeyCode::E))
		{
			SendPacket(37001000, SkillInputAction::Pressed, tick);
			pressedSkillId = 37001000;
		}
		else if (Input::GetKeyReleased(Input::KeyCode::E))
		{
			SendPacket(37001000, SkillInputAction::Released, tick);
			if (pressedSkillId == 37001000)
				pressedSkillId = 0;
		}
	}
	SH_USER_API void SkillManager::TickFixed(uint64_t tick)
	{
		if (!player->IsLocal())
			return;

		UseSkill(pressedSkillId, tick);
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
		const auto& pos = gameObject.transform->GetWorldPosition();
		for (auto effect : state->skill->GetEffects())
		{
			if (effect == nullptr)
				continue;
			SH_INFO("effect!");
			effect->SpawnProjectile(world, player, pos.x, pos.y, player->GetMovement()->IsRight());
		}
		ApplyCooldown(id);
		lastState = state;
		SH_INFO_FORMAT("Use skill: {}", id);
	}
	void SkillManager::SendPacket(SkillId skillId, SkillInputAction action, uint64_t tick)
	{
		static MapleClient& client = *MapleClient::GetInstance();

		SkillUsingPacket packet{};
		packet.seq = seq++;
		packet.tick = tick;
		packet.userUUID = client.GetUser().GetUserUUID();
		packet.skillId = skillId;
		packet.action = action;
		client.SendPacket(packet);
	}
}//namespace
