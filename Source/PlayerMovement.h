#pragma once
#include "Export.h"

#include "Game/Component/Component.h"
#include "Game/Vector.h"

namespace sh::game
{
	class RigidBody;
	class PlayerMovement : public Component
	{
		COMPONENT(PlayerMovement, "user")
	public:
		SH_USER_API PlayerMovement(GameObject& owner);

		SH_USER_API void Awake() override;
		SH_USER_API void Start() override;
		SH_USER_API void BeginUpdate() override;
		SH_USER_API void FixedUpdate() override;
		SH_USER_API void Update() override;
	private:
		void LimitSpeed();
	private:
		PROPERTY(rb)
		RigidBody* rb = nullptr;
		PROPERTY(speed)
		float speed = 10.0f;
		PROPERTY(force)
		float force = 2.0f;
		PROPERTY(bounceVelocity)
		float bounceVelocity = 10.0f;

		game::Vec3 startPos;

		bool isGround = false;
		bool bBouncing = false;
	};
}