#include "Mob/MobMovement.h"

// 공용
namespace sh::game
{
	MobMovement::MobMovement(GameObject& owner) :
		FootholdMovement(owner)
	{
	}
	SH_USER_API void MobMovement::FixedUpdate()
	{
		StepMovement();
	}
}//namespace