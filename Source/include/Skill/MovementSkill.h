#pragma once
#include "Export.h"
#include "Skill/Skill.h"

#include <cstdint>
namespace sh::game
{
	class MovementSkill : public Skill
	{
		SRPO(MovementSkill)
	public:
		enum class MoveType : uint8_t
		{
			Impulse = 0,
			Teleport = 1
		};
		enum class ApplyPhase : uint8_t
		{
			Start = 0,
			Active = 1,
			Recovery = 2
		};
	public:
		SH_USER_API auto GetMoveType() const -> MoveType { return static_cast<MoveType>(moveType); }
		SH_USER_API auto GetApplyPhase() const -> ApplyPhase { return static_cast<ApplyPhase>(applyPhase); }
		SH_USER_API auto GetMoveX() const -> float { return moveX; }
		SH_USER_API auto GetMoveY() const -> float { return moveY; }
		SH_USER_API auto GetMaxDistance() const -> float { return maxDistance; }
		SH_USER_API auto IsUseFacing() const -> bool { return bUseFacing; }
		SH_USER_API auto IsRequireGround() const -> bool { return bRequireGround; }
	private:
		PROPERTY(moveType)
		uint32_t moveType = static_cast<uint32_t>(MoveType::Impulse);
		PROPERTY(applyPhase)
		uint32_t applyPhase = static_cast<uint32_t>(ApplyPhase::Active);
		PROPERTY(moveX)
		float moveX = 0.f;
		PROPERTY(moveY)
		float moveY = 0.f;
		PROPERTY(maxDistance)
		float maxDistance = 0.f;
		PROPERTY(bUseFacing)
		bool bUseFacing = true;
		PROPERTY(bRequireGround)
		bool bRequireGround = false;
	};
}//namespace
