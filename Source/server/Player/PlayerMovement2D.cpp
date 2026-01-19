#include "Player/PlayerMovement2D.h"
#include "CollisionTag.hpp"

#include "Game/GameObject.h"

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
			[&](const network::PacketEvent& evt)
			{
				if (evt.packet->GetId() == PlayerInputPacket::ID)
					ProcessInputPacket(static_cast<const PlayerInputPacket&>(*evt.packet));
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
			rigidBody->GetCollider()->SetAllowCollisions(tag::groundTag | tag::itemTag);
			SetPriority(-1);
		}
	}
	SH_USER_API void PlayerMovement2D::Start()
	{
		server = MapleServer::GetInstance();
		assert(server != nullptr);
		server->bus.Subscribe(packetEventSubscriber);
	}
	SH_USER_API void PlayerMovement2D::BeginUpdate()
	{
		if (!core::IsValid(rigidBody))
			return;
		ProcessInput();
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
		const auto& pos = gameObject.transform->GetWorldPosition();
		const auto v = rigidBody->GetLinearVelocity();

		const glm::vec2 serverPos{ pos.x, pos.y };
		const glm::vec2 serverVel = { v.x, v.y };

		if (bFirst)
		{
			bSend = true;
			bFirst = false;
		}
		else
		{
			float posDiff = glm::length(serverPos - lastSent.pos);
			float velDiff = glm::length(serverVel - lastSent.vel);

			// 새로 들어온 입력이 있었고, 그걸 처리했다면 즉시 송신
			if (lastProcessedSeq != lastSent.seq)
				bSend = true;
			// 임계값 옵션
			//else if (posDiff > 0.05f || velDiff > 0.05f)
			//	send = true;
		}

		// 2 ticks 마다 강제 송신 (60fps기준 1초에 30번)
		if (serverTick++ >= 2)
			bSend = true;

		if (bSend)
		{
			PlayerStatePacket packet;

			packet.px = pos.x;
			packet.py = pos.y;
			packet.vx = v.x;
			packet.vy = v.y;
			packet.lastProcessedInputSeq = lastProcessedSeq;
			packet.playerUUID = player->GetUserUUID();
			packet.serverTick = serverTick;
			packet.timestamp = lastTick;
			packet.bGround = bGround;
			packet.floor = floorY;
			packet.bProne = lastInput.bProne;
			packet.bLock = bLock;
			packet.bRight = player->IsRight();
			server->BroadCast(packet);

			lastSent.pos = serverPos;
			lastSent.vel = serverVel;
			lastSent.seq = lastProcessedSeq;

			serverTick = 0;
			bSend = false;
		}
	}
	SH_USER_API void PlayerMovement2D::OnCollisionEnter(Collider& collider)
	{
		if (!floor.IsValid())
			return;
		if (floor->GetCollider() == &collider)
			bGround = true;
	}
	SH_USER_API void PlayerMovement2D::OnTriggerEnter(Collider& collider)
	{
	}
	SH_USER_API void PlayerMovement2D::Lock()
	{
		bLock = true;
		if (rigidBody != nullptr)
		{
			auto vel = rigidBody->GetLinearVelocity();
			rigidBody->SetLinearVelocity({ 0.0f, vel.y, 0.0f });
		}
		bSend = true;
	}
	SH_USER_API void PlayerMovement2D::Unlock()
	{
		bLock = false;
		bSend = true;
	}

	void PlayerMovement2D::ProcessInputPacket(const PlayerInputPacket& packet)
	{
		if (!core::IsValid(rigidBody))
			return;
		if (player->GetUserUUID() != packet.user)
			return;
		if (lastInput.seq >= packet.seq) // 과거 패킷임
			return;

		if (lastInput.xMove != packet.inputX)
			lastInput.xMove = packet.inputX;
		if (lastInput.bJump != packet.bJump)
			lastInput.bJump = packet.bJump;
		if (lastInput.bProne != packet.bProne)
			lastInput.bProne = packet.bProne;
		lastInput.seq = packet.seq;
	}
	void PlayerMovement2D::ProcessInput()
	{
		yVelocity = rigidBody->GetLinearVelocity().y;
		if (bGround)
		{
			if (!lastInput.bProne)
			{
				if (lastInput.xMove == 0 || bLock)
					xVelocity = 0;
				else
					xVelocity = std::clamp(lastInput.xMove * speed, -speed, speed);

				if (lastInput.bJump && !bLock)
				{
					yVelocity = jumpSpeed;
					bGround = false;
				}
			}
			else
			{
				xVelocity = 0.0f;
			}
		}
		else
		{
			float airSpeed = 0.1f;
			if (lastInput.xMove != 0 && !bLock)
			{
				float targetSpeed = lastInput.xMove * speed;
				xVelocity = glm::mix(xVelocity, targetSpeed, airSpeed);
			}
		}
		// 방향
		if (!lastInput.bProne && !bLock)
		{
			if (lastInput.xMove > 0.0f)
				player->SetRight(true);
			else if (lastInput.xMove < 0.0f)
				player->SetRight(false);
		}

		rigidBody->SetLinearVelocity({ xVelocity, yVelocity, 0.f });

		lastProcessedSeq = lastInput.seq;
		lastTick = lastInput.tick;
	}
}//namespace