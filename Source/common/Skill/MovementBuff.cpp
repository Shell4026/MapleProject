#include "Skill/MovementBuff.h"
#include "Player/Player.h"
#include "Player/PlayerMovement.h"
#include "Skill/Skill.h"

namespace sh::game
{
	SH_USER_API void MovementBuff::OnApply(Player& player, const Skill& skill) const
	{
		PlayerMovement* const movement = player.GetMovement();
		if (!core::IsValid(movement))
			return;

		if (bRequireGround && !movement->IsGround())
			return;

		float appliedMoveX = moveX;
		if (bUseFacing && !movement->IsRight())
			appliedMoveX = -appliedMoveX;

		if (GetMoveType() == MoveType::Impulse)
			movement->ApplySkillImpulse(appliedMoveX, moveY);
		else
			movement->ApplySkillTeleport(appliedMoveX, moveY);
	}
}//namespace
