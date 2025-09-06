#include "PlayerMovement2D.h"

#include "Game/GameObject.h"
#include "Game/Input.h"

#include <algorithm>
namespace sh::game
{
	PlayerMovement2D::PlayerMovement2D(GameObject& owner) :
		Component(owner)
	{
	}
	SH_USER_API void PlayerMovement2D::Start()
	{
		SH_INFO("start");
		world.GetPhysWorld()->SetGravity({ 0.f, -20.f, 0.f });
		if (core::IsValid(rigidBody))
		{
			rigidBody->SetAxisLock({ 1, 1, 1 });
			SetPriority(rigidBody->GetPriority() - 1);
		}
	}
	SH_USER_API void PlayerMovement2D::BeginUpdate()
	{
		const auto& pos = gameObject.transform->GetWorldPosition();
		yVelocity = rigidBody->GetLinearVelocity().y;

		float xMove = 0.f;
		if (Input::GetKeyDown(Input::KeyCode::Right))
		{
			xMove += 1;
			if (core::IsValid(anim))
			{
				anim->bRight = true;
			}
		}
		if (Input::GetKeyDown(Input::KeyCode::Left))
		{
			xMove += -1;
			if (core::IsValid(anim))
				anim->bRight = false;
		}

		if (bGround)
		{
			if (xMove == 0)
				xVelocity = 0;
			else
				xVelocity = std::clamp(xMove * speed, -speed, speed);

			if (core::IsValid(anim))
			{
				if (xVelocity != 0)
					anim->SetPose(PlayerAnimation::Pose::Walk);
				else
					anim->SetPose(PlayerAnimation::Pose::Idle);
			}
		}
		else
		{
			float airSpeed = 0.1f;
			if (xMove != 0)
			{
				// 현재 속도에서 목표 속도로 보간
				float targetSpeed = xMove * speed;
				xVelocity = glm::mix(xVelocity, targetSpeed, airSpeed);
			}
			if (core::IsValid(anim))
				anim->SetPose(PlayerAnimation::Pose::Jump);
		}
		

		if (Input::GetKeyPressed(Input::KeyCode::F))
			Jump();

		rigidBody->SetLinearVelocity({ xVelocity, yVelocity, 0.f });

		phys::Ray ray{ {pos.x, pos.y + 0.02f, pos.z}, Vec3{0.0f, -1.0f, 0.f}, 0.1f };
		auto hitOpt = world.GetPhysWorld()->RayCast(ray);
		if (hitOpt.has_value())
		{
			floorY = hitOpt.value().hitPoint.y;
			floor = RigidBody::GetRigidBodyFromHandle(hitOpt.value().rigidBodyHandle);
			bGround = true;
		}
		else
		{
			floorY = -1000.0f;
			bGround = false;
		}
	}
	SH_USER_API void PlayerMovement2D::Update()
	{
		const auto& pos = gameObject.transform->GetWorldPosition();
		if (pos.y < floorY)
		{
			gameObject.transform->SetPosition(pos.x, floorY, pos.z);
			gameObject.transform->UpdateMatrix();
		}
	}
	void PlayerMovement2D::Jump()
	{
		yVelocity = jumpSpeed;
	}
}//namespace