#include "PlayerMovement2D.h"
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

#if SH_SERVER
		packetEventSubscriber.SetCallback
		(
			[&](const PacketEvent& evt)
			{
				Endpoint ep = { evt.senderIp, evt.senderPort };
				if (server->GetUser(ep) == nullptr)
					return;
				if (evt.packet->GetId() == PlayerInputPacket::ID)
					ProcessInputPacket(static_cast<const PlayerInputPacket&>(*evt.packet), ep);
			}
		);
#else
		
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
#endif
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
#if SH_SERVER
		server = MapleServer::GetInstance();
		assert(server != nullptr);
		server->bus.Subscribe(packetEventSubscriber);
#else
		client = MapleClient::GetInstance();
		assert(client != nullptr);
		client->bus.Subscribe(packetEventSubscriber);

		serverPos = gameObject.transform->GetWorldPosition();
#endif
	}
	SH_USER_API void PlayerMovement2D::BeginUpdate()
	{
#if SH_SERVER
		if (!core::IsValid(rigidBody))
			return;
		ProcessInput();
#else
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
#endif
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
#if SH_SERVER
		
		const auto& pos = gameObject.transform->GetWorldPosition();
		const auto v = rigidBody->GetLinearVelocity();

		const glm::vec2 serverPos{ pos.x, pos.y };
		const glm::vec2 serverVel = { v.x, v.y };

		static bool bFirst = true;

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
			packet.playerUUID = player->GetUserUUID().ToString();
			packet.serverTick = serverTick;
			packet.timestamp = lastTick;
			packet.bGround = bGround;
			packet.floor = floorY;
			packet.bProne = lastInput.bProne;
			packet.bLock = bLock;
			server->BroadCast(packet);

			lastSent.pos = serverPos;
			lastSent.vel = serverVel;
			lastSent.seq = lastProcessedSeq;

			serverTick = 0;
			bSend = false;
		}
#else
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
#endif
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
#if SH_SERVER
		bSend = true;
#endif
	}
	SH_USER_API void PlayerMovement2D::Unlock()
	{
		bLock = false;
#if SH_SERVER
		bSend = true;
#endif
	}
	SH_USER_API auto PlayerMovement2D::IsLock() const -> bool
	{
		return bLock;
	}
#if SH_SERVER
	void PlayerMovement2D::ProcessInputPacket(const PlayerInputPacket& packet, const Endpoint& endpoint)
	{
		if (!core::IsValid(rigidBody))
			return;
		if (packet.playerUUID != player->GetUserUUID().ToString())
			return;
		if (lastInput.seq >= packet.seq) // 과거 패킷임
			return;

		if (lastInput.xMove != packet.inputX)
			lastInput.xMove = packet.inputX;
		if (lastInput.bJump != packet.bJump)
			lastInput.bJump = packet.bJump;
		if (lastInput.bProne != packet.bProne)
			lastInput.bProne = packet.bProne;
		lastInput.tick = packet.timestamp;
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
		rigidBody->SetLinearVelocity({ xVelocity, yVelocity, 0.f });

		lastProcessedSeq = lastInput.seq;
		lastTick = lastInput.tick;
	}
#else
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
			packet.playerUUID = client->GetUser().GetUserUUID().ToString();
			packet.timestamp = tick;
			packet.bProne = bProne;

			lastSent = std::move(packet);

			client->SendPacket(lastSent);

			SH_INFO_FORMAT("send (tick:{})", tick);
		}
	}
	void PlayerMovement2D::ProcessStatePacket(const PlayerStatePacket& packet)
	{
		if (packet.playerUUID != player->GetUserUUID().ToString())
			return;

		if (!core::IsValid(rigidBody))
			return;

		const auto& pos = gameObject.transform->GetWorldPosition();

		serverPos = { packet.px, packet.py };
		serverVel = { packet.vx, packet.vy };

		bGround = packet.bGround;
		floorY = packet.floor;
		bProne = packet.bProne;

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

		if (xInput > 0)
			anim->bRight = true;
		else if (xInput < 0)
			anim->bRight = false;

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

				if (xVelocity >= 0.5f)
					anim->bRight = true;
				else if (xVelocity <= -0.5f)
					anim->bRight = false;
			}
			else
				anim->SetPose(PlayerAnimation::Pose::Prone);
		}
		else
			anim->SetPose(PlayerAnimation::Pose::Jump);
	}
#endif
}//namespace