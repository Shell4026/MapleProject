#include "PlayerMovement2D.h"

#include "Game/GameObject.h"
#include "Game/Input.h"

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
					ProcessStatePacket(static_cast<const PlayerStatePacket&>(*evt.packet), player->IsLocal());
				}
			}
		);
#endif
	}
	SH_USER_API void PlayerMovement2D::Awake()
	{
		world.GetPhysWorld()->SetGravity({ 0.f, -20.f, 0.f });
		if (core::IsValid(rigidBody))
		{
			rigidBody->SetAxisLock({ 1, 1, 1 });
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

		if (player->IsLocal())
			ProcessLocalInput();
		else
			ProcessRemoteAnim();

		++tick;
#endif
	}
	SH_USER_API void PlayerMovement2D::Update()
	{
		{
			const auto& pos = gameObject.transform->GetWorldPosition();
			if (pos.y < floorY)
			{
				gameObject.transform->SetPosition(pos.x, floorY, pos.z);
				gameObject.transform->UpdateMatrix();
			}
		}
#if SH_SERVER
		// 이 시점에서 물리 계산이 끝났음 (FixedUpdate->Update)
		const auto& pos = gameObject.transform->GetWorldPosition();
		const auto v = rigidBody->GetLinearVelocity();

		const glm::vec2 serverPos{ pos.x, pos.y };
		const glm::vec2 serverVel = { v.x, v.y };

		static bool bFirst = true;

		bool bSend = false;
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
			SH_INFO_FORMAT("foor: {}", floorY);

			server->BroadCast(packet);

			lastSent.pos = serverPos;
			lastSent.vel = serverVel;
			lastSent.seq = lastProcessedSeq;

			serverTick = 0;
		}
#endif
	}
#if SH_SERVER
	void PlayerMovement2D::ProcessInputPacket(const PlayerInputPacket& packet, const Endpoint& endpoint)
	{
		if (!core::IsValid(rigidBody))
			return;
		if (packet.playerUUID != player->GetUserUUID().ToString())
			return;

		if (lastInput.xMove != packet.inputX)
			lastInput.xMove = packet.inputX;
		if (packet.bJump)
			bJump = true;
		lastInput.tick = packet.timestamp;
		lastInput.seq = packet.seq;
	}
	void PlayerMovement2D::ProcessInput()
	{
		const auto& pos = gameObject.transform->GetWorldPosition();
		const phys::Ray ray{ {pos.x, pos.y + 0.02f, pos.z}, Vec3{0.0f, -1.0f, 0.f}, 0.1f };
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

		yVelocity = rigidBody->GetLinearVelocity().y;
		if (bGround)
		{
			if (lastInput.xMove == 0)
				xVelocity = 0;
			else
				xVelocity = std::clamp(lastInput.xMove * speed, -speed, speed);

			if (bJump)
			{
				yVelocity = jumpSpeed;
				bJump = false;
			}
		}
		else
		{
			float airSpeed = 0.1f;
			if (lastInput.xMove != 0)
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

		float xMove = 0.f;
		if (Input::GetKeyDown(Input::KeyCode::Right))
		{
			xMove += 1;
			if (core::IsValid(anim))
				anim->bRight = true;
		}
		if (Input::GetKeyDown(Input::KeyCode::Left))
		{
			xMove += -1;
			if (core::IsValid(anim))
				anim->bRight = false;
		}
		bool bJump = false;
		if (Input::GetKeyPressed(Input::KeyCode::F))
			bJump = true;

		// 움직임 예측 코드
		if (bGround)
		{
			if (xMove == 0)
				xVelocity = 0;
			else
				xVelocity = std::clamp(xMove * speed, -speed, speed);

			if (bJump)
				yVelocity = jumpSpeed;

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
		rigidBody->SetLinearVelocity({ xVelocity, yVelocity, 0.f });

		bool bInputChanged = false;
		if (pendingInputs.empty())
			bInputChanged = true;
		else
			bInputChanged = (xMove != pendingInputs.back().inputX) || (bJump != pendingInputs.back().bJump);

		if (bInputChanged)
		{
			PlayerInputPacket packet{};
			packet.inputX = xMove;
			packet.bJump = bJump;
			packet.seq = inputSeqCounter++;
			packet.playerUUID = client->GetUser().GetUserUUID().ToString();
			packet.timestamp = tick;

			client->SendPacket(packet);
			pendingInputs.push_back(std::move(packet));
		}
	}
	void PlayerMovement2D::ProcessStatePacket(const PlayerStatePacket& packet, bool bLocal)
	{
		if (packet.playerUUID != player->GetUserUUID().ToString())
			return;

		if (!core::IsValid(rigidBody))
			return;

		const glm::vec2 serverPos{ packet.px, packet.py };
		const glm::vec2 serverVel{ packet.vx, packet.vy };
		bGround = packet.bGround;
		floorY = packet.floor;

		xVelocity = serverVel.x;
		yVelocity = serverVel.y;
		if (bLocal)
		{
			const uint64_t processedTick = packet.timestamp;

			glm::vec2 corrected = serverPos;

			// 서버에서 처리한 마지막 입력만 남기고 앞에 입력들을 지움
			while (!pendingInputs.empty() && pendingInputs.front().seq <= packet.lastProcessedInputSeq)
				pendingInputs.pop_front();

			if (inputSeqCounter != packet.lastProcessedInputSeq)
			{
				const uint64_t tickDif = tick - processedTick;
				SH_INFO_FORMAT("tickDif: {}", tickDif);
				for (int i = 0; i < tickDif; ++i)
				{
					yVelocity -= 20.f * world.FIXED_TIME;
					corrected.x += xVelocity * world.FIXED_TIME;
					corrected.y += yVelocity * world.FIXED_TIME;
				}
			}
			rigidBody->SetLinearVelocity({ xVelocity, yVelocity, 0.f });
			gameObject.transform->SetPosition(serverPos.x, serverPos.y, 0.f);
			gameObject.transform->UpdateMatrix();
			rigidBody->ResetPhysicsTransform();
		}
		else
		{
			rigidBody->SetLinearVelocity({ xVelocity, yVelocity, 0.f });
			gameObject.transform->SetPosition(serverPos.x, serverPos.y, 0.f);
			gameObject.transform->UpdateMatrix();
			rigidBody->ResetPhysicsTransform();
		}
	}
	void PlayerMovement2D::ProcessRemoteAnim()
	{
		if (!core::IsValid(anim))
			return;

		if (yVelocity != 0)
			anim->SetPose(PlayerAnimation::Pose::Jump);

		if (bGround)
		{
			if (xVelocity == 0)
				anim->SetPose(PlayerAnimation::Pose::Idle);
			else
			{
				if (xVelocity >= 1.0f)
					anim->bRight = true;
				else if (xVelocity <= -1.0f)
					anim->bRight = false;
				anim->SetPose(PlayerAnimation::Pose::Walk);
			}
		}
	}
#endif
}//namespace