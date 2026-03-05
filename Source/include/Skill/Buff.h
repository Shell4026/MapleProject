#pragma once
#include "Export.h"

#include "Game/ScriptableObject.h"

#include <cstdint>
namespace sh::game
{
	class Player;
	class Skill;
	class Buff : public ScriptableObject
	{
		SRPO(Buff)
	public:
		enum class ApplyPhase : uint8_t
		{
			Start = 0,
			Active = 1,
			Recovery = 2
		};
	public:
		SH_USER_API virtual void OnApply(Player& player, const Skill& skill) const {};

		SH_USER_API auto GetApplyPhase() const -> ApplyPhase { return static_cast<ApplyPhase>(applyPhase); }
	private:
		PROPERTY(applyPhase)
		uint32_t applyPhase = static_cast<uint32_t>(ApplyPhase::Active);
	};
}//namespace
