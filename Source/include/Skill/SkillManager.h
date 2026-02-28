#pragma once
#include "Export.h"
#include "Player/IPlayerTickable.h"
#include "Skill.h"
#include "Packet/SkillUsingPacket.hpp"

#include "Core/SContainer.hpp"

#include "Game/Component/Component.h"
#include "Game/Input.h"

#include <cstdint>
#include <deque>
#include <unordered_map>
namespace sh::game
{
	class Player;
	class SkillManager : public Component, public IPlayerTickable
	{
		COMPONENT(SkillManager, "user")
	public:
		using SkillId = uint32_t;
		struct SkillState
		{
			Skill* skill = nullptr;
			uint32_t skillId = 0;
			float counterMs = 0.f;
			float cooldownRemainingMs = 0.f;
			enum class State
			{
				Wait,
				Start,
				Active,
				Recovery
			} state = State::Wait;
		};
	public:
		SH_USER_API SkillManager(GameObject& owner);

		SH_USER_API void Awake() override;

		SH_USER_API void TickBegin(uint64_t tick) override;
		SH_USER_API void TickFixed(uint64_t tick) override;

		SH_USER_API void RegisterSkill(Skill& skill);
		SH_USER_API void UnRegisterSkill(SkillId id);

		SH_USER_API auto CanUse(SkillId id) const -> bool;
		SH_USER_API void ApplyCooldown(SkillId id);
		SH_USER_API auto IsOnCooldown(SkillId id) const -> bool;
		SH_USER_API auto GetSkill(SkillId id) const -> Skill*;
		SH_USER_API auto GetSkillState(SkillId id) const -> const SkillState*;
		SH_USER_API auto GetLastSkillState() const -> SkillState* { return lastState; }
#if SH_SERVER
		SH_USER_API void ProcessPacket(const SkillUsingPacket& packet);
#else
		SH_USER_API void SetKeyBinding(Input::KeyCode keycode, SkillId skill);
#endif
	private:
		auto GetSkillState(SkillId id) -> SkillState*;
		void UseSkill(SkillId id, uint64_t tick);
		void UpdateState();
	private:
		PROPERTY(player, core::PropertyOption::sobjPtr)
		Player* player = nullptr;

		std::unordered_map<SkillId, std::size_t> skillStateIdxs;
		std::vector<SkillState> skillStates;

		SkillState* lastState = nullptr;
#if SH_SERVER
		struct PendingSkill
		{
			uint32_t seq = 0;
			SkillId skillId = 0;
			uint64_t applyServerTick = 0;
		} currentSkill;
		uint32_t lastSeq = 0;
		uint64_t offset = 0;
		bool bOffsetInit = false;
		std::deque<PendingSkill> pendingSkills;
#else
		uint32_t seq = 1;
		std::unordered_map<Input::KeyCode, SkillId> keybindings;
		struct InputState
		{
			SkillId skillId = 0;
		} lastInput;

		bool bPendingSend = false;
#endif
	};
}//namespace
