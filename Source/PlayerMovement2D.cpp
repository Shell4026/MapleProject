#include "PlayerMovement2D.h"

#include "Game/GameObject.h"
#include "Game/Input.h"

#include <algorithm>
namespace sh::game
{
	PlayerMovement2D::PlayerMovement2D(GameObject& owner) :
		Component(owner)
	{
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
	SH_USER_API void PlayerMovement2D::Start()
	{
		world.GetPhysWorld()->SetGravity({ 0.f, -20.f, 0.f });
		if (core::IsValid(rigidBody))
		{
			rigidBody->SetAxisLock({ 1, 1, 1 });
			SetPriority(rigidBody->GetPriority() - 1);
		}
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
		const auto& pos = gameObject.transform->GetWorldPosition();
		const auto v = rigidBody->GetLinearVelocity();

		const glm::vec2 serverPos{ pos.x, pos.y };
		const glm::vec2 serverVel = { v.x, v.y };

		bool send = false;
		if (playerData.lastSent.bFirst)
		{
			send = true;
			playerData.lastSent.bFirst = false;
		}
		else
		{
			float posDiff = glm::length(serverPos - playerData.lastSent.pos);
			float velDiff = glm::length(serverVel - playerData.lastSent.vel);

			// 새로 들어온 입력이 있었고, 그걸 처리했다면 즉시 송신
			if (playerData.lastProcessedSeq != playerData.lastSent.seq)
				send = true;
			// 임계값 옵션
			//else if (posDiff > 0.05f || velDiff > 0.05f)
			//	send = true;
		}

		// 3 ticks 마다 강제 송신 (60fps기준 1초에 20번)
		if (serverTick++ >= 3)
			send = true;

		if (send)
		{
			PlayerStatePacket packet;

			packet.px = pos.x;
			packet.py = pos.y;
			packet.vx = v.x;
			packet.vy = v.y;
			packet.lastProcessedInputSeq = playerData.lastProcessedSeq;
			packet.playerUUID = player->GetUserUUID().ToString();
			packet.serverTick = serverTick;
			packet.timestamp = 0;

			server->BroadCast(packet);

			playerData.lastSent.pos = serverPos;
			playerData.lastSent.vel = serverVel;
			playerData.lastSent.seq = playerData.lastProcessedSeq;

			serverTick = 0;
		}
#else
		if (!core::IsValid(player))
			return;

		if (player->IsLocal())
		{
			SH_INFO_FORMAT("Im local {}", (void*)player);
			ProcessLocalInput();
		}
		else
			SH_INFO_FORMAT("Im remote {}", (void*)player);
		++tick;
#endif
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
#if SH_SERVER
	void PlayerMovement2D::ProcessInputPacket(const PlayerInputPacket& packet, const Endpoint& endpoint)
	{
		if (!core::IsValid(rigidBody))
			return;
		if (packet.playerUUID != player->GetUserUUID().ToString())
			return;

		PlayerData::InputState input{};
		input.xMove = packet.inputX;
		input.jump = packet.jump;
		input.seq = packet.seq;

		yVelocity = rigidBody->GetLinearVelocity().y;
		if (bGround)
		{
			if (input.xMove == 0)
				xVelocity = 0;
			else
				xVelocity = std::clamp(input.xMove * speed, -speed, speed);

			if (input.jump)
				yVelocity = jumpSpeed;
		}
		else
		{
			float airSpeed = 0.1f;
			if (input.xMove != 0)
			{
				float targetSpeed = input.xMove * speed;
				xVelocity = glm::mix(xVelocity, targetSpeed, airSpeed);
			}
		}
		rigidBody->SetLinearVelocity({ xVelocity, yVelocity, 0.f });

		playerData.lastInput = input;
		playerData.lastProcessedSeq = input.seq;
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
			if (playerData.lastInput.xMove == 0)
				xVelocity = 0;
			else
				xVelocity = std::clamp(playerData.lastInput.xMove * speed, -speed, speed);

			if (playerData.lastInput.jump)
				yVelocity = jumpSpeed;
		}
		else
		{
			float airSpeed = 0.1f;
			if (playerData.lastInput.xMove != 0)
			{
				float targetSpeed = playerData.lastInput.xMove * speed;
				xVelocity = glm::mix(xVelocity, targetSpeed, airSpeed);
			}
		}
		rigidBody->SetLinearVelocity({ xVelocity, yVelocity, 0.f });
	}
#else
	void PlayerMovement2D::ProcessLocalInput()
	{
		const auto& pos = gameObject.transform->GetWorldPosition();
		yVelocity = rigidBody->GetLinearVelocity().y;

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

		bool inputChanged = (xMove != lastSent.xMove) || (bJump != lastSent.bJump);

		if (inputChanged)
		{
			PlayerInputPacket packet{};
			packet.inputX = xMove;
			packet.jump = bJump;
			packet.seq = ++lastSent.inputSeqCounter;
			packet.playerUUID = client->GetUser().GetUserUUID().ToString();
			packet.timestamp = 0;

			client->SendPacket(packet);
			pendingInputs.push_back(std::move(packet));

			lastSent.xMove = xMove;
			lastSent.bJump = bJump;
		}
	}
	void PlayerMovement2D::ProcessStatePacket(const PlayerStatePacket& packet)
	{
		if (player->GetUserUUID().ToString() != packet.playerUUID)
			return;

		const glm::vec2 serverPos{ packet.px, packet.py };
		const glm::vec2 serverVel{ packet.vx, packet.vy };

		// 서버에서 처리한 마지막 입력만 남기고 앞에 입력들을 지움
		while (!pendingInputs.empty() && pendingInputs.front().seq <= packet.lastProcessedInputSeq)
			pendingInputs.pop_front();

		const auto& curPos = gameObject.transform->GetWorldPosition();
		float mix = 1.0f;
		float dif = glm::length(serverPos - glm::vec2{ curPos });
		mix = 0.9f;
		//SH_INFO_FORMAT("len: {}, mix: {}", dif, mix);
		auto corrected = glm::mix(glm::vec2{ curPos.x, curPos.y }, serverPos, mix);
		gameObject.transform->SetPosition(corrected.x, corrected.y, 0.f);
		gameObject.transform->UpdateMatrix();

		rigidBody->ResetPhysicsTransform();

		xVelocity = serverVel.x;
		yVelocity = serverVel.y;
		if (core::IsValid(rigidBody))
			rigidBody->SetLinearVelocity({ xVelocity, yVelocity, 0.f });

		// 서버에서 입력된 처리 후의 움직임 예측
		for (const auto& input : pendingInputs)
		{
			if (input.inputX == 0)
				xVelocity = 0;
			else
				xVelocity = std::clamp(input.inputX * speed, -speed, speed);

			if (input.jump && bGround)
				yVelocity = jumpSpeed;
			yVelocity -= 20.f * world.deltaTime;

			if (core::IsValid(rigidBody))
				rigidBody->SetLinearVelocity({ xVelocity, yVelocity, 0.f });
		}

	}
#endif
}//namespace