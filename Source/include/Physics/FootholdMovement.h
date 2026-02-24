#pragma once
#include "Export.h"
#include "Foothold.h"

#include "Game/Component/Component.h"
#include "Game/Vector.h"
namespace sh::game
{
	class FootholdMovement : public Component
	{
		COMPONENT(FootholdMovement, "user")
	public:
		SH_USER_API FootholdMovement(GameObject& owner);

		SH_USER_API void Start() override;

		SH_USER_API void StepMovement();
		SH_USER_API void SetExpectedGround();

		SH_USER_API void SetFoothold(const Foothold& foothold) { this->foothold = &foothold; }
		SH_USER_API void SetVelocity(float vx, float vy) { velX = vx; velY = vy; }
		SH_USER_API void SetVelocity(const Vec2& vel) { velX = vel.x; velY = vel.y; }
		SH_USER_API void SetSpeed(float speed) { this->speed = speed; }
		SH_USER_API void SetJumpSpeed(float speed) { jumpSpeed = speed; }
		SH_USER_API void SetIsGround(bool bGround) { this->bGround = bGround; }

		SH_USER_API auto GetSpeed() const -> float { return speed; }
		SH_USER_API auto GetJumpSpeed() const -> float { return jumpSpeed; }
		SH_USER_API auto GetVelocity() const -> Vec2 { return Vec2{ velX, velY }; }
		SH_USER_API auto GetFoothold() const -> const Foothold* { return foothold; }
		SH_USER_API auto IsGround() const -> bool { return bGround; }
	private:
		void ApplyGravity();
		void ApplyPos();
		void CheckGround();
		void MoveOnGround();
		void ClampPos(float unit = 100.f);
	protected:
		float velX = 0.f;
		float velY = 0.f;
	private:
		constexpr static float G = 20.f;

		PROPERTY(foothold)
		const Foothold* foothold = nullptr;
		PROPERTY(speed)
		float speed = 1.25f;
		PROPERTY(jumpSpeed)
		float jumpSpeed = 5.55f;
		PROPERTY(maxFallSpeed)
		float maxFallSpeed = 6.7f;
		PROPERTY(maxStepHeight)
		float maxStepHeight = 0.2f;

		Foothold::Contact ground;
		float offset = 0.1f;

		bool bGround = false;
	};
}//namespace