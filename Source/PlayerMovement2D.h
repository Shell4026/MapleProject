#pragma once
#include "Export.h"
#include "PlayerAnimation.h"

#include "Core/SContainer.hpp"

#include "Game/Component/Component.h"
#include "Game/Component/RigidBody.h"
namespace sh::game
{
	class PlayerMovement2D : public Component
	{
		COMPONENT(PlayerMovement2D, "user")
	public:
		SH_USER_API PlayerMovement2D(GameObject& owner);

		SH_USER_API void Start() override;
		SH_USER_API void BeginUpdate() override;
		SH_USER_API void Update() override;
	private:
		void Jump();
	private:
		PROPERTY(speed)
		float speed = 1.5f;
		PROPERTY(jumpSpeed)
		float jumpSpeed = 5.5f;
		PROPERTY(rigidBody)
		RigidBody* rigidBody = nullptr;
		PROPERTY(anim)
		PlayerAnimation* anim = nullptr;

		core::SObjWeakPtr<RigidBody> floor;

		float xVelocity = 0.f;
		float yVelocity = 0.f;

		float floorY = -1000.0f;

		bool bGround = false;
	};
}//namespace