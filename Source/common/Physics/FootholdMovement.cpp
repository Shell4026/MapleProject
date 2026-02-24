#include "Physics/FootholdMovement.h"

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
		ApplyGravity();
		ApplyPos();
		CheckGround();
		ClampPos();
	}
	SH_USER_API void FootholdMovement::SetExpectedGround()
	{
		const auto& p = gameObject.transform->GetWorldPosition();
		ground = foothold->GetExpectedFallContact({ p.x, p.y + offset });
	}
	void FootholdMovement::ApplyGravity()
	{
		if (bGround)
			return;
		const float dt = world.FIXED_TIME;
		velY -= G * dt;
		velY = std::clamp(velY, -maxFallSpeed, maxFallSpeed);
	}
	void FootholdMovement::ApplyPos()
	{
		const float dt = world.FIXED_TIME;
		auto pos = gameObject.transform->GetWorldPosition();

		if (!bGround)
		{
			pos.x += velX * dt;
			ground = foothold->GetExpectedFallContact({ pos.x, pos.y + offset });
			pos.y += velY * dt;

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
			if (pos.y <= ground.pos.y && velY < 0.f)
			{
				gameObject.transform->SetWorldPosition({ pos.x, ground.pos.y, pos.z });
				velY = 0.f;
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

		const float targetX = pos.x + velX * dt;

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
				bGround = false;
				pos.x = targetX;
				gameObject.transform->SetWorldPosition(pos);
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
				bGround = false;
				pos.x = targetX;
				gameObject.transform->SetWorldPosition(pos);
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