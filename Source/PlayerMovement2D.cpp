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
				if (evt.packet->GetId() == PlayerInputPacket::ID)
				{
					ProcessInputPacket(static_cast<const PlayerInputPacket&>(*evt.packet));
				}
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
		PlayerStatePacket packet;
		const auto& pos = gameObject.transform->GetWorldPosition();
		packet.px = pos.x; 
		packet.py = pos.y;
		if (core::IsValid(rigidBody))
		{
			auto v = rigidBody->GetLinearVelocity();
			packet.vx = v.x; 
			packet.vy = v.y; 
		}
		packet.lastProcessedInputSeq = lastProcessedSeq;
		packet.playerUUID = player->GetUserUUID().ToString();
		packet.serverTick = ++serverTick;
		packet.timestamp = 0;

		server->BroadCast(packet);

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
#else
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
				Jump();

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

		PlayerInputPacket packet{};
		packet.inputX = xMove;
		packet.jump = bJump;
		packet.seq = nextInputSeq++;
		packet.playerUUID = client->GetUser().GetUUID().ToString();
		packet.timestamp = 0;
		client->SendPacket(packet);
		pendingInputs.push_back(std::move(packet));
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
	void PlayerMovement2D::Jump()
	{
		yVelocity = jumpSpeed;
	}
	void PlayerMovement2D::ProcessStatePacket(const PlayerStatePacket& packet)
	{
#if !SH_SERVER
		if (packet.playerUUID != client->GetUser().GetUUID().ToString()) 
			return;

		const glm::vec2 serverPos{ packet.px, packet.py };
		const glm::vec2 serverVel{ packet.vx, packet.vy };

		// 서버에서 처리한 마지막 입력만 남기고 앞에 입력들을 지움
		while (!pendingInputs.empty() && pendingInputs.front().seq <= packet.lastProcessedInputSeq)
			pendingInputs.pop_front();

		const auto& curPos = gameObject.transform->GetWorldPosition();
		float mix = 1.0f;
		float dif = glm::length(serverPos - glm::vec2{ curPos });
		mix = 0.75f;
		SH_INFO_FORMAT("len: {}, mix: {}", dif, mix);
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
			//float dt = world.fixedDeltaTime + world.deltaTime;
			//while(dt >= world.FIXED_TIME)
			//{
			//	yVelocity -= 20.f * world.FIXED_TIME;
			//	dt -= world.FIXED_TIME;
			//}

			if (core::IsValid(rigidBody))
				rigidBody->SetLinearVelocity({ xVelocity, yVelocity, 0.f });
		}
#endif
	}
	void PlayerMovement2D::ProcessInputPacket(const PlayerInputPacket& packet)
	{
#if SH_SERVER
		if (packet.playerUUID != player->GetUserUUID().ToString()) 
			return;

		yVelocity = rigidBody->GetLinearVelocity().y;

		lastProcessedSeq = packet.seq;
		float xMove = packet.inputX;
		if (bGround)
		{
			if (xMove == 0) 
				xVelocity = 0;
			else 
				xVelocity = std::clamp(xMove * speed, -speed, speed);

			if (packet.jump) 
				yVelocity = jumpSpeed;
		}
		else
		{
			float airSpeed = 0.1f;
			if (xMove != 0)
			{
				float targetSpeed = xMove * speed;
				xVelocity = glm::mix(xVelocity, targetSpeed, airSpeed);
			}
		}

		if (core::IsValid(rigidBody))
			rigidBody->SetLinearVelocity({ xVelocity, yVelocity, 0.f });
#endif
	}
}//namespace