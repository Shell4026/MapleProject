#include "Player/PlayerMovement.h"
#include "MapleWorld.h"
#include "MapleServer.h"
#include "Packet/PlayerInputPacket.hpp"

#include "Game/World.h"
namespace sh::game
{
	// 서버 코드
	SH_USER_API void PlayerMovement::Awake()
	{
		if (player == nullptr)
			SH_ERROR("Player is nullptr!");

		// 이벤트들은 BeginUpdate전에 이뤄짐
		packetSubscriber.SetCallback(
			[this](const network::PacketEvent& evt)
			{
				if (evt.packet->GetId() == PlayerInputPacket::ID)
				{
					auto& packet = static_cast<const PlayerInputPacket&>(*evt.packet);
					if (player->GetUserUUID() == packet.user)
						ProcessInput(packet);
				}
			}
		);
		MapleServer::GetInstance()->bus.Subscribe(packetSubscriber);

		foothold = player->GetCurrentWorld()->GetFoothold();
	}
	SH_USER_API void PlayerMovement::BeginUpdate()
	{
		if (lastInput.xMove > 0)
			velX = speed;
		else if (lastInput.xMove < 0)
			velX = -speed;
		else
			velX = 0.f;

		if (lastInput.bJump && bGround)
		{
			velY = jumpSpeed;
			bGround = false;
		}
	}
	SH_USER_API void PlayerMovement::FixedUpdate()
	{
		ApplyGravity();
		ApplyPos();
		CheckGround();
		++tick;
	}
	SH_USER_API void PlayerMovement::Update()
	{
		if (sendTick++ >= 2)
			bSend = true;

		if (bSend)
		{
			static auto& server = *MapleServer::GetInstance();
			auto& pos = gameObject.transform->GetWorldPosition();

			PlayerStatePacket packet;

			packet.lastProcessedInputSeq = lastInput.seq;
			packet.serverTick = tick;
			packet.px = pos.x;
			packet.py = pos.y;
			packet.vx = velX;
			packet.vy = velY;
			packet.playerUUID = player->GetUserUUID();
			packet.bGround = bGround;
			packet.bProne = lastInput.bProne;
			packet.bLock = bInputLock;
			packet.bRight = player->IsRight();
			server.BroadCast(packet);

			bSend = false;
			sendTick = 0;
		}
	}
	void PlayerMovement::ProcessInput(const PlayerInputPacket& packet)
	{
		if (lastInput.seq >= packet.seq)
			return;

		lastInput.seq = packet.seq;
		lastInput.tick = packet.tick;
		lastInput.xMove = packet.inputX;
		lastInput.bJump = packet.bJump;
		lastInput.bProne = packet.bProne;

		bSend = true;
	}
}//namespace