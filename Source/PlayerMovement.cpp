#include "PlayerMovement.h"

#include "Game/Component/RigidBody.h"
#include "Game/GameObject.h"
#include "Game/Input.h"

#include "Core/Logger.h"

#include "Physics/Ray.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/norm.hpp"

//class GroundRaycastCallback : public reactphysics3d::RaycastCallback
//{
//public:
//	bool hit = false;
//	float hitFraction = 0.0f;
//	reactphysics3d::Vector3 worldPoint;
//	reactphysics3d::Vector3 worldNormal;
//
//	/// 첫 번째 히트에서 바로 멈추도록 false 반환
//	auto notifyRaycastHit(const reactphysics3d::RaycastInfo& info) -> reactphysics3d::decimal override
//	{
//		hit = true;
//		hitFraction = info.hitFraction;
//		worldPoint = info.worldPoint;
//		worldNormal = info.worldNormal;
//		return 0.0f;
//	}
//};

namespace sh::game
{
	void PlayerMovement::LimitSpeed()
	{
		auto linearV = rb->GetLinearVelocity();
		glm::vec3 v{ linearV.x, 0.f, linearV.z };
		if (glm::length2(v) > speed * speed)
		{
			v = glm::normalize(v);
			v *= speed;
			v.y = linearV.y;

			rb->SetLinearVelocity(v);
		}
		v = rb->GetAngularVelocity();
		if (glm::length2(v) > speed * speed)
		{
			v = glm::normalize(v);
			v *= speed;
			rb->SetAngularVelocity(v);
		}
	}
	PlayerMovement::PlayerMovement(GameObject& owner) :
		Component(owner)
	{
	}
	SH_USER_API void PlayerMovement::Awake()
	{
		Super::Awake();
		if (rb == nullptr)
			rb = gameObject.GetComponent<RigidBody>();

		SH_INFO("Awake?");
	}
	SH_USER_API void PlayerMovement::Start()
	{
		SH_INFO("Start?");
		rb->SetAngularDamping(1.0f);

		startPos = gameObject.transform->GetWorldPosition();
	}
	SH_USER_API void PlayerMovement::BeginUpdate()
	{
		if (!core::IsValid(rb))
			return;

		if (Input::GetKeyDown(Input::KeyCode::W))
		{
			rb->AddWorldTorque(game::Vec3{ -speed, 0.f, 0.f });
			rb->AddWorldForce(game::Vec3{ 0.f, 0.f, -speed  });
		}
		if (Input::GetKeyDown(Input::KeyCode::A))
		{
			rb->AddWorldTorque(game::Vec3{ 0.f, 0.f, speed });
			rb->AddWorldForce(game::Vec3{ -speed, 0.f, 0.f });
		}
		if (Input::GetKeyDown(Input::KeyCode::S))
		{
			rb->AddWorldTorque(game::Vec3{ speed, 0.f, 0.f });
			rb->AddWorldForce(game::Vec3{ 0.f, 0.f, speed });
		}
		if (Input::GetKeyDown(Input::KeyCode::D))
		{
			rb->AddWorldTorque(game::Vec3{ 0.f, 0.f, -speed });
			rb->AddWorldForce(game::Vec3{ speed, 0.f, 0.f });
		}
	}
	SH_USER_API void PlayerMovement::FixedUpdate()
	{
		auto pos = gameObject.transform->GetWorldPosition();
		if (pos.y < -10.0f) // 바닥 아래로 떨어지면 리셋
		{
			gameObject.transform->SetWorldPosition(startPos);
			rb->SetAngularVelocity(game::Vec3{ 0.f, 0.f, 0.f });
			rb->SetLinearVelocity(game::Vec3{ 0.f, 0.f, 0.f });
		}
		const phys::Ray ray{ gameObject.transform->position, { 0.f, -1.0f, 0.f }, 0.2f };
		isGround = world.GetPhysWorld()->RayCastHit(ray);
		if (isGround)
		{
			auto v = rb->GetLinearVelocity();
			if (v.y < 0)
			{
				v.y = bounceVelocity;
				rb->SetLinearVelocity(v);
			}
			//rb->AddForce({ 0.f, 10.f, 0.f });
		}
		LimitSpeed();
	}
	SH_USER_API void PlayerMovement::Update()
	{
	}
}//namespace