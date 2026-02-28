#include "Phys/FootholdMovement.h"

#include "Game/World.h"
namespace sh::game
{
	FootholdMovement::FootholdMovement(GameObject& owner) :
		Component(owner)
	{
	}
	SH_USER_API void FootholdMovement::Start()
	{
		if (foothold != nullptr)
		{
			auto& pos = gameObject.transform->GetWorldPosition();
			ground = foothold->GetExpectedFallContact({ pos.x, pos.y });
		}
	}
	SH_USER_API void FootholdMovement::StepMovement()
	{
		ApplyForce();
		ApplyPos();
		CheckGround();
		ClampPos();
	}
	SH_USER_API void FootholdMovement::SetExpectedGround()
	{
		const auto& p = gameObject.transform->GetWorldPosition();
		ground = foothold->GetExpectedFallContact({ p.x, p.y + offset });
	}
	void FootholdMovement::ApplyForce()
	{
		const float dt = world.FIXED_TIME;
		const bool sameXdirForce = vel.x * force.x > 0;
		if (!sameXdirForce) // 감속
			vel.x += force.x * dt;
		else
		{
			if (std::abs(vel.x) < speed)
			{
				vel.x += force.x * dt;
				vel.x = std::clamp(vel.x, -speed, speed);
			}
		}
		vel.x += impulse.x;

		if (!bGround) // 중력
			vel.y -= G * dt;
		vel.y = std::clamp(vel.y, -maxFallSpeed, maxFallSpeed);
		const bool sameYdirForce = vel.y * force.y > 0;
		if (!sameYdirForce)
			vel.y += force.y * dt;
		else
		{
			if (std::abs(vel.y) < maxFallSpeed)
			{
				vel.y += force.y * dt;
				vel.y = std::clamp(vel.y, -maxFallSpeed, maxFallSpeed);
			}
		}
		vel.y += impulse.y;

		if (bGround)
		{
			const float v = std::abs(vel.x);
			if (v - groundDrag * dt < 0.f)
				vel.x = 0.f;
			else
				vel.x -= std::copysignf(groundDrag * dt, vel.x);
		}
		else
		{
			const float v = std::abs(vel.x);
			if (v - airDrag * dt < 0.f)
				vel.x = 0.f;
			else
				vel.x -= std::copysignf(airDrag * dt, vel.x);
		}

		force.x = 0.f;
		force.y = 0.f;
		impulse.x = 0.f;
		impulse.y = 0.f;
	}
	void FootholdMovement::ApplyPos()
	{
		const float dt = world.FIXED_TIME;
		auto pos = gameObject.transform->GetWorldPosition();

		if (!bGround)
		{
			pos.x += vel.x * dt;
			ground = foothold->GetExpectedFallContact({ pos.x, pos.y + offset });
			pos.y += vel.y * dt;

			gameObject.transform->SetWorldPosition(pos);
		}
		else
			MoveOnGround();
		gameObject.transform->UpdateMatrix();
	}
	void FootholdMovement::CheckGround()
	{
		if (bGround)
			return;
		const auto& pos = gameObject.transform->GetWorldPosition();
		if (ground.pathIdx != -1)
		{
			if (pos.y <= ground.pos.y && vel.y < 0.f)
			{
				gameObject.transform->SetWorldPosition({ pos.x, ground.pos.y, pos.z });
				vel.y = 0.f;
				bGround = true;
			}
		}
		gameObject.transform->UpdateMatrix();
	}
	void FootholdMovement::MoveOnGround()
	{
		const float dt = world.FIXED_TIME;

		const Foothold::Path* const pathPtr = foothold->GetPath(ground.pathIdx);
		if (pathPtr == nullptr)
		{
			bGround = false;
			return;
		}
		auto pos = gameObject.transform->GetWorldPosition();

		const auto& p0 = pathPtr->points[ground.point0];
		const auto& p1 = pathPtr->points[ground.point1];

		const float targetX = pos.x + vel.x * dt;

		// 현재 선 이탈
		if (targetX > std::max(p0.x, p1.x))
		{
			if (ground.point1 < (int)pathPtr->points.size() - 1)
			{
				ground.point0++;
				ground.point1++;
			}
			else // 끝이 끊긴 지형
			{
				if (bCliffFall)
				{
					bGround = false;
					pos.x = targetX;
					gameObject.transform->SetWorldPosition(pos);
				}
				else
				{
					pos.x = std::max(p0.x, p1.x);
					gameObject.transform->SetWorldPosition(pos);
				}
				return;
			}
		}
		else if (targetX < std::min(p0.x, p1.x))
		{
			if (ground.point0 > 0)
			{
				ground.point0--;
				ground.point1--;
			}
			else // 끝이 끊긴 지형
			{
				if (bCliffFall)
				{
					bGround = false;
					pos.x = targetX;
					gameObject.transform->SetWorldPosition(pos);
				}
				else
				{
					pos.x = std::min(p0.x, p1.x);
					gameObject.transform->SetWorldPosition(pos);
				}
				return;
			}
		}
		const auto& np0 = pathPtr->points[ground.point0];
		const auto& np1 = pathPtr->points[ground.point1];

		const float px = (np1.x - np0.x);

		float targetY = np1.y;
		if (px > 0.001f)
		{
			const float t = (targetX - np0.x) / px; // 삼각형의 가로 비율 (SAS닮음)
			targetY = t * (np1.y - np0.y) + np0.y;
		}
		const float dy = targetY - pos.y;
		if (dy > maxStepHeight)
			return;
		else if (dy < -maxStepHeight)
		{
			bGround = false;
			pos.x = targetX;
			gameObject.transform->SetWorldPosition(pos);
			return;
		}
		gameObject.transform->SetWorldPosition({ targetX, targetY, pos.z });
	}
	void FootholdMovement::ClampPos(float unit)
	{
		auto pos = gameObject.transform->GetWorldPosition();
		pos.x = std::roundf(pos.x * unit) / unit;
		pos.y = std::roundf(pos.y * unit) / unit;
		gameObject.transform->SetWorldPosition(pos);
		gameObject.transform->UpdateMatrix();
	}
}//namespace