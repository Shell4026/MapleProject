#include "Player/PlayerMovement.h"
#include "MapleWorld.h"
#include "Packet/PlayerInputPacket.hpp"

#include "Game/World.h"
namespace sh::game
{
	// 서버 사이드
	SH_USER_API void PlayerMovement::Awake()
	{
		if (player == nullptr)
			SH_ERROR("Player is nullptr!");
	}
	SH_USER_API void PlayerMovement::TickBegin(uint64_t tick)
	{
	}
	SH_USER_API void PlayerMovement::TickFixed(uint64_t tick)
	{
		while (!inputs.empty() && inputs.front().applyServerTick <= tick)
		{
			currentState = inputs.front();
			inputs.pop_front();
		}

		if (bInputLock)
		{
			vel.x = 0.f;
		}
		else if (!currentState.bProne)
		{
			if (currentState.xMove > 0)
			{
				AddForce(14.f, 0.f);
				//vel.x = GetSpeed();
				bRight = true;
			}
			else if (currentState.xMove < 0)
			{
				AddForce(-14.f, 0.f);
				//vel.x = -GetSpeed();
				bRight = false;
			}
			//else
			//	vel.x = 0.f;

			if (currentState.bJump && IsGround())
			{
				vel.y = GetJumpSpeed();
				SetIsGround(false);
			}
		}
		else
			vel.x = 0.f;

		StepMovement();
	}
	SH_USER_API void PlayerMovement::TickUpdate(uint64_t tick)
	{
		if (sendTick++ >= 2)
			bPendingSend = true;

		if (bPendingSend)
		{
			static auto& server = *MapleServer::GetInstance();
			auto& pos = gameObject.transform->GetWorldPosition();

			PlayerStatePacket packet;

			packet.lastProcessedInputSeq = currentState.seq;
			packet.serverTick = tick;
			packet.clientTickAtState = currentState.clientTick + (tick - currentState.applyServerTick);

			packet.px = pos.x;
			packet.py = pos.y;
			packet.vx = vel.x;
			packet.vy = vel.y;
			packet.playerUUID = player->GetUUID();
			packet.bGround = IsGround();
			packet.bProne = currentState.bProne;
			packet.bLock = bInputLock;
			packet.bRight = IsRight();
			server.BroadCast(packet);

			bPendingSend = false;
			sendTick = 0;
		}
	}
	SH_USER_API void PlayerMovement::ProcessInput(const PlayerInputPacket& packet)
	{
		const uint64_t tick = player != nullptr ? player->GetTick() : 0;
		if (!bOffsetInit)
		{
			offset = tick - packet.tick + 5;
			bOffsetInit = true;
		}
		else
		{
			const uint64_t newOffset = tick - packet.tick + 5;
			offset = (offset * 9 + newOffset) / 10;
		}
		// 패킷 이벤트들은 BeginUpdate전에 이뤄짐
		if (!inputs.empty() && inputs.back().seq >= packet.seq)
			return;

		//SH_INFO_FORMAT("recv seq: {}, tick: {}, serverTick: {}, offset: {}", packet.seq, packet.tick, tick, offset);

		InputState input{};
		input.seq = packet.seq;
		input.clientTick = packet.tick;
		input.applyServerTick = packet.tick + offset;
		input.xMove = packet.inputX;
		input.bJump = packet.bJump;
		input.bProne = bProne = packet.bProne;
		inputs.push_back(input);

		bPendingSend = true;
	}
}//namespace
