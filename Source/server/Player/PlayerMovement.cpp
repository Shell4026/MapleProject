#include "Player/PlayerMovement.h"
#include "World/MapleWorld.h"
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
		while (!recvStates.empty() && recvStates.front().applyServerTick <= tick)
		{
			applyServerTick = recvStates.front().applyServerTick;
			clientTick = recvStates.front().clientTick;
			state = recvStates.front().state;
			recvStates.pop_front();
		}

		if (state.bLock)
		{
			vel.x = 0.f;
		}
		else if (!state.bProne)
		{
			if (state.xMove > 0)
			{
				AddForce(14.f, 0.f);
				//vel.x = GetSpeed();
				bRight = true;
			}
			else if (state.xMove < 0)
			{
				AddForce(-14.f, 0.f);
				//vel.x = -GetSpeed();
				bRight = false;
			}
			//else
			//	vel.x = 0.f;

			if (state.bJump && IsGround())
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

			packet.lastProcessedInputSeq = state.seq;
			packet.serverTick = tick;
			packet.clientTickAtState = clientTick + (tick - applyServerTick);

			packet.px = pos.x;
			packet.py = pos.y;
			packet.vx = vel.x;
			packet.vy = vel.y;
			packet.playerUUID = player->GetUUID();
			packet.bGround = IsGround();
			packet.bLock = state.bLock;
			packet.bUp = state.bUp;
			packet.bProne = state.bProne;
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
		if (!recvStates.empty() && recvStates.back().state.seq >= packet.seq)
			return;

		//SH_INFO_FORMAT("recv seq: {}, tick: {}, serverTick: {}, offset: {}", packet.seq, packet.tick, tick, offset);

		State state{};
		state.seq = packet.seq;
		state.xMove = packet.inputX;
		state.bJump = packet.bJump;
		state.bUp = packet.bUp;
		state.bProne = packet.bProne;

		RecvState recv{};
		recv.clientTick = packet.tick;
		recv.applyServerTick = packet.tick + offset;
		recv.state = state;
		recvStates.push_back(recv);

		bPendingSend = true;
	}
}//namespace
