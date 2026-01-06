#include "Player/PlayerMovement2D.h"
#include "CollisionTag.hpp"

#include "Game/GameObject.h"
#include "Game/Input.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/norm.hpp"

#include <algorithm>
namespace sh::game
{
	PlayerMovement2D::PlayerMovement2D(GameObject& owner) :
		Component(owner)
	{
		// 이벤트들은 BeginUpdate전에 호출됨
		packetEventSubscriber.SetCallback
		(
			[&](const PacketEvent& evt)
			{
				if (evt.packet->GetId() == PlayerStatePacket::ID)
				{
					ProcessStatePacket(static_cast<const PlayerStatePacket&>(*evt.packet));
				}
			}
		);
	}
	SH_USER_API void PlayerMovement2D::Awake()
	{
		world.GetPhysWorld()->SetGravity({ 0.f, -20.f, 0.f });
		if (rigidBody != nullptr)
		{
			rigidBody->SetAngularLock({ 1, 1, 1 });
			rigidBody->SetAxisLock({ 0, 0, 1 });
			rigidBody->GetCollider()->SetCollisionTag(tag::entityTag);
			rigidBody->GetCollider()->SetAllowCollisions(tag::groundTag);
			SetPriority(-1);
		}
	}
	SH_USER_API void PlayerMovement2D::Start()
	{
		client = MapleClient::GetInstance();
		assert(client != nullptr);
		client->bus.Subscribe(packetEventSubscriber);

		serverPos = gameObject.transform->GetWorldPosition();
	}
	SH_USER_API void PlayerMovement2D::BeginUpdate()
	{
		if (!core::IsValid(player))
			return;

		++tick;

		if (player->IsLocal())
		{
			ProcessLocalInput();
			ProcessLocalAnim(lastSent.inputX);
		}
		else
		{
			ProcessRemoteAnim();
		}
	}
	SH_USER_API void PlayerMovement2D::FixedUpdate()
	{
		if (!core::IsValid(rigidBody))
			return;

		const game::Vec3 v = rigidBody->GetLinearVelocity();
		if (rigidBody->GetLinearVelocity().y > jumpSpeed)
			rigidBody->SetLinearVelocity({ v.x, jumpSpeed, v.z });
		else if(rigidBody->GetLinearVelocity().y < -maxFallSpeed)
			rigidBody->SetLinearVelocity({ v.x, -maxFallSpeed, v.z });
		const game::Vec3 pos = rigidBody->GetPhysicsPosition();

		phys::Ray ray{ {pos.x, pos.y + 0.4f, pos.z}, Vec3{0.0f, -1.0f, 0.f}, rayDistance };
		auto hits = world.GetPhysWorld()->RayCast(ray, tag::groundTag);
		if (!hits.empty())
		{
			floorY = hits.front().hitPoint.y;
			floor = RigidBody::GetRigidBodyFromHandle(hits.front().rigidBodyHandle);
		}
		else
		{
			floor.Reset();
			floorY = -1000.0f;
			bGround = false;
		}
	}
	SH_USER_API void PlayerMovement2D::Update()
	{
		// 이 시점에서 물리 계산이 끝났음 (FixedUpdate->Update)
		{
			const auto& pos = gameObject.transform->GetWorldPosition();
			if (pos.y < floorY)
				gameObject.transform->SetPosition(pos.x, floorY, pos.z);
			gameObject.transform->UpdateMatrix();
		}
		// 단순 보간
		glm::vec2 corrected = serverPos;
		glm::vec2 correctedVel = serverVel;
		
		const float difSqr = glm::distance2(glm::vec2{ gameObject.transform->GetWorldPosition() }, serverPos);
		if (difSqr < 1.0f * 1.0f)
		{
			corrected = glm::mix(glm::vec2{ gameObject.transform->GetWorldPosition() }, serverPos, 0.2f);
			correctedVel = glm::mix(glm::vec2{ rigidBody->GetLinearVelocity() }, serverVel, 1.0f);
		}
		gameObject.transform->SetWorldPosition({ corrected.x, corrected.y, player->IsLocal() ? 0.025f : 0.02f });
		gameObject.transform->UpdateMatrix();
		rigidBody->SetLinearVelocity({ correctedVel.x, correctedVel.y, 0.f });
		rigidBody->ResetPhysicsTransform();
	}
	SH_USER_API void PlayerMovement2D::OnCollisionEnter(Collider& collider)
	{
		if (!floor.IsValid())
			return;
		if (floor->GetCollider() == &collider)
			bGround = true;
	}
	SH_USER_API void PlayerMovement2D::Lock()
	{
		bLock = true;
		if (rigidBody != nullptr)
		{
			auto vel = rigidBody->GetLinearVelocity();
			rigidBody->SetLinearVelocity({ 0.0f, vel.y, 0.0f });
		}
	}
	SH_USER_API void PlayerMovement2D::Unlock()
	{
		bLock = false;
	}

	void PlayerMovement2D::ProcessLocalInput()
	{
		const auto& pos = gameObject.transform->GetWorldPosition();
		yVelocity = rigidBody->GetLinearVelocity().y;

		float xInput = 0.f;
		bool bJump = false;
		bool bProne = false;
		if (Input::GetKeyDown(Input::KeyCode::Down))
			bProne = true;
		if (!bProne)
		{
			if (Input::GetKeyDown(Input::KeyCode::Right))
				xInput += 1;
			if (Input::GetKeyDown(Input::KeyCode::Left))
				xInput += -1;
			if (Input::GetKeyDown(Input::KeyCode::F))
				bJump = true;
		}
		// 움직임 예측 코드
		if (bGround)
		{
			if (xInput == 0 || bLock)
				xVelocity = 0;
			else
				xVelocity = std::clamp(xInput * speed, -speed, speed);

			if (bJump && !bLock)
			{
				yVelocity = jumpSpeed;
				bGround = false;
			}
		}
		else
		{
			float airSpeed = 0.1f;
			if (xInput != 0 && !bLock)
			{
				// 현재 속도에서 목표 속도로 보간
				float targetSpeed = xInput * speed;
				xVelocity = glm::mix(xVelocity, targetSpeed, airSpeed);
			}
		}
		rigidBody->SetLinearVelocity({ xVelocity, yVelocity, 0.f });

		bool bInputChanged = false;
		bInputChanged = (xInput != lastSent.inputX) || (bJump != lastSent.bJump || (bProne != lastSent.bProne));

		if (bInputChanged)
		{
			PlayerInputPacket packet{};
			packet.inputX = xInput;
			packet.bJump = bJump;
			packet.seq = inputSeqCounter++;
			packet.playerUUID = client->GetUser().GetUserUUID();
			packet.timestamp = tick;
			packet.bProne = bProne;

			lastSent = std::move(packet);

			client->SendPacket(lastSent);

			SH_INFO_FORMAT("send (tick:{})", tick);
		}
	}
	void PlayerMovement2D::ProcessStatePacket(const PlayerStatePacket& packet)
	{
		if (player->GetUserUUID() != packet.playerUUID)
			return;

		if (!core::IsValid(rigidBody))
			return;

		const auto& pos = gameObject.transform->GetWorldPosition();

		serverPos = { packet.px, packet.py };
		serverVel = { packet.vx, packet.vy };

		bGround = packet.bGround;
		floorY = packet.floor;
		bProne = packet.bProne;
		player->SetRight(packet.bRight);

		xVelocity = serverVel.x;
		yVelocity = serverVel.y;

		if (packet.bLock)
			Lock();
		else
			Unlock();
	}
	void PlayerMovement2D::ProcessLocalAnim(float xInput)
	{
		if (!core::IsValid(anim) || bLock)
			return;

		if (xInput == 0.0f)
		{
			if (bProne)
				anim->SetPose(PlayerAnimation::Pose::Prone);
			else
				anim->SetPose(PlayerAnimation::Pose::Idle);
		}
		else
			anim->SetPose(PlayerAnimation::Pose::Walk);

		if (!bGround)
			anim->SetPose(PlayerAnimation::Pose::Jump);

		// 방향
		if (!bProne && !bLock)
		{
			if (xInput > 0)
				player->SetRight(true);
			else if (xInput < 0)
				player->SetRight(false);
		}
	}
	void PlayerMovement2D::ProcessRemoteAnim()
	{
		if (!core::IsValid(anim) || bLock)
			return;

		if (bGround)
		{
			if (!bProne)
			{
				if (std::abs(xVelocity) > 0.5f)
					anim->SetPose(PlayerAnimation::Pose::Walk);
				else
					anim->SetPose(PlayerAnimation::Pose::Idle);
			}
			else
				anim->SetPose(PlayerAnimation::Pose::Prone);
		}
		else
			anim->SetPose(PlayerAnimation::Pose::Jump);
	}
}//namespace