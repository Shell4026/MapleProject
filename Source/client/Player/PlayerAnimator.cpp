#include "Player/PlayerAnimator.h"
#include "Player/PlayerMovement.h"
namespace sh::game
{
	PlayerAnimator::PlayerAnimator(GameObject& owner) :
		Animator(owner)
	{
	}
	SH_USER_API void PlayerAnimator::Awake()
	{
		Super::Awake();
		if (movement == nullptr)
			SH_ERROR("movement is nullptr!");
	}
	SH_USER_API void PlayerAnimator::Update()
	{
		const auto& vel = movement->GetVelocity();
		if (!movement->IsGround())
			SetState(2);
		else if (std::abs(vel.x) > 0.1f)
			SetState(1);
		else
			SetState(0);

		Super::Update();
	}
}//namespace