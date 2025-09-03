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
		world.GetPhysWorld()->SetGravity({ 0.f, -20.f, 0.f });
		if (core::IsValid(rigidBody))
		{
			rigidBody->SetAxisLock({ 1, 1, 1 });
		}
	}
	SH_USER_API void PlayerMovement2D::BeginUpdate()
	{
		const auto& pos = gameObject.transform->GetWorldPosition();
		yVelocity = rigidBody->GetLinearVelocity().y;
		if (Input::GetKeyDown(Input::KeyCode::Right))
		{
			xVelocity += speed;
		}
		if (Input::GetKeyDown(Input::KeyCode::Left))
		{
			xVelocity -= speed;
		}
		if (Input::GetKeyReleased(Input::KeyCode::Right) && xVelocity > 0)
			xVelocity = 0;
		if (Input::GetKeyReleased(Input::KeyCode::Left) && xVelocity < 0)
			xVelocity = 0;

		xVelocity = std::clamp(xVelocity, -speed, speed);

		if (Input::GetKeyPressed(Input::KeyCode::F))
			Jump();

		rigidBody->SetLinearVelocity({ xVelocity, yVelocity, 0.f });

		phys::Ray ray{ pos, Vec3{0.0f, -1.0f, 0.f}, 0.5f };
		auto hitOpt = world.GetPhysWorld()->RayCast(ray);
		if (hitOpt.has_value())
			floorY = hitOpt.value().hitPoint.y;
		else
			floorY = -1000.0f;
	}
	SH_USER_API void PlayerMovement2D::FixedUpdate()
	{
		
	}
	SH_USER_API void PlayerMovement2D::LateUpdate()
	{
		const auto& pos = gameObject.transform->GetWorldPosition();
		if (pos.y < floorY)
		{
			gameObject.transform->SetWorldPosition(pos.x, floorY, pos.z);
			gameObject.transform->UpdateMatrix();
		}
	}
	void PlayerMovement2D::Jump()
	{
		yVelocity = jumpSpeed;
	}
}//namespace