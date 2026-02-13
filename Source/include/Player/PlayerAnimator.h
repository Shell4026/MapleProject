#pragma once
#include "Export.h"
#include "Animator.h"

namespace sh::game
{
	class PlayerMovement;
	class PlayerAnimator : public Animator
	{
		COMPONENT(PlayerAnimator, "user")
	public:
		SH_USER_API PlayerAnimator(GameObject& owner);

		SH_USER_API void Awake() override;
		SH_USER_API void Update() override;
	private:
		void DecideState();
		void ApplyRight();
	private:
		PROPERTY(movement, core::PropertyOption::sobjPtr)
		PlayerMovement* movement = nullptr;
	};
}//namespace