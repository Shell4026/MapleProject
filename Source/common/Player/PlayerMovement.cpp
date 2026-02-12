#include "Player/PlayerMovement.h"
#include "MapleWorld.h"

#include "Game/World.h"
#include "Game/Input.h"

namespace sh::game
{
	// 공용 코드
	PlayerMovement::PlayerMovement(GameObject& owner) :
		Component(owner)
	{
		offset = 0.1f;
	}
	SH_USER_API void PlayerMovement::Start()
	{
		const auto& pos = gameObject.transform->GetWorldPosition();
		ground = foothold->GetExpectedFallContact({ pos.x, pos.y });
	}
	void PlayerMovement::StepMovement()
	{
		ApplyGravity();
		ApplyPos();
		CheckGround();
		ClampPos();
	}
	void PlayerMovement::ApplyGravity()
	{
		if (bGround)
			return;
		const float dt = world.FIXED_TIME;
		velY -= G * dt;
		velY = std::clamp(velY, -maxFallSpeed, maxFallSpeed);
	}
	void PlayerMovement::ApplyPos()
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
	void PlayerMovement::CheckGround()
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
	void PlayerMovement::MoveOnGround()
	{
		const float dt = world.FIXED_TIME;

		auto path = foothold->GetPath(ground.pathIdx);
		if (path == nullptr) 
		{ 
			bGround = false; 
			return; 
		}
		auto pos = gameObject.transform->GetWorldPosition();

		const auto& p0 = path->points[ground.point0];
		const auto& p1 = path->points[ground.point1];

		const float targetX = pos.x + velX * dt;

		// 현재 선 이탈
		if (targetX > std::max(p0.x, p1.x))
		{
			if (ground.point1 < (int)path->points.size() - 1)
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
		const auto& np0 = path->points[ground.point0];
		const auto& np1 = path->points[ground.point1];

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
	void PlayerMovement::ClampPos()
	{
		auto pos = gameObject.transform->GetWorldPosition();
		pos.x = std::roundf(pos.x * 100.0f) / 100.0f;
		pos.y = std::roundf(pos.y * 100.0f) / 100.0f;
		gameObject.transform->SetWorldPosition(pos);
		gameObject.transform->UpdateMatrix();
	}
}//namespace