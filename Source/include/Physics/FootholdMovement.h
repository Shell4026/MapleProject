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
		SH_USER_API void SetVelocity(float vx, float vy) { vel.x = vx; vel.y = vy; }
		SH_USER_API void SetVelocity(const Vec2& vel) { this->vel = vel; }
		SH_USER_API void SetSpeed(float speed) { this->speed = speed; }
		SH_USER_API void SetJumpSpeed(float speed) { jumpSpeed = speed; }
		SH_USER_API void SetIsGround(bool bGround) { this->bGround = bGround; }
		SH_USER_API void SetCliffFall(bool bCliffFall) { this->bCliffFall = bCliffFall; }
		SH_USER_API void AddForce(const Vec2& force) { this->force = force; };
		SH_USER_API void AddForce(float fx, float fy) { force.x = fx; force.y = fy; };
		/// @brief 속도 제한에 걸리지 않는 힘
		/// @param force 힘 (질량 = 1취급)
		SH_USER_API void AddImpulse(const Vec2& force) { impulse = force; }
		SH_USER_API void AddImpulse(float fx, float fy) { impulse.x = fx; impulse.y = fy; }

		SH_USER_API auto GetSpeed() const -> float { return speed; }
		SH_USER_API auto GetJumpSpeed() const -> float { return jumpSpeed; }
		SH_USER_API auto GetVelocity() const -> const Vec2& { return vel; }
		SH_USER_API auto GetFoothold() const -> const Foothold* { return foothold; }
		SH_USER_API auto IsGround() const -> bool { return bGround; }
		SH_USER_API auto IsCliffFall() const -> bool { return bCliffFall; }
	private:
		void ApplyForce();
		void ApplyPos();
		void CheckGround();
		void MoveOnGround();
		void ClampPos(float unit = 100.f);
	protected:
		Vec2 force{ 0.f, 0.f };
		Vec2 vel{ 0.f, 0.f };
		Vec2 impulse{ 0.f, 0.f };
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
		PROPERTY(groundDrag)
		float groundDrag = 8.f;
		PROPERTY(airDrag)
		float airDrag = 1.f;
		Foothold::Contact ground;
		float offset = 0.1f;

		bool bGround = false;
		PROPERTY(bCliffFall)
		bool bCliffFall = true;
	};
}//namespace