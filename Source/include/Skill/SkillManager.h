#pragma once
#include "Export.h"
#include "Skill.h"
#include "Packet/SkillUsingPacket.hpp"

#include "Core/SContainer.hpp"

#include "Game/Component/Component.h"
#include "Game/Input.h"

#include <cstdint>
#include <unordered_map>
namespace sh::game
{
	class Player;
	class SkillManager : public Component
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
		SH_USER_API void BeginUpdate() override;

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
		void UseSkill(SkillId id);
		void UpdateState();
	private:
		PROPERTY(player, core::PropertyOption::sobjPtr)
		Player* player = nullptr;
#if SH_SERVER
		uint32_t lastSeq = 0;
#else
		uint32_t seq = 1;
		std::unordered_map<Input::KeyCode, SkillId> keybindings;
#endif
		std::unordered_map<SkillId, std::size_t> skillStateIdxs;
		std::vector<SkillState> skillStates;

		SkillState* lastState = nullptr;
	};
}//namespace