#include "Player/PlayerMovement.h"
#include "World/MapleWorld.h"
#include "Packet/PlayerInputPacket.hpp"
#include "Skill/SkillManager.h"

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
		bJumpTriggeredThisTick = false;
		bLandedThisTick = false;
		const bool bLockPrev = state.bLock;

		while (!recvStates.empty() && recvStates.front().applyServerTick <= tick)
		{
			applyServerTick = recvStates.front().applyServerTick;
			clientTick = recvStates.front().clientTick;
			state = recvStates.front().state;
			state.bLock = bLockPrev;
			recvStates.pop_front();
		}

		if (state.bLock)
		{
			state.xMove = 0;
			state.bJump = false;
			state.bUp = false;
			state.bProne = false;
		}

		const bool bWasGround = IsGround();
		if (!state.bProne)
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
				bJumpTriggeredThisTick = true;
			}
		}

		StepMovement();
		if (!bWasGround && IsGround())
			bLandedThisTick = true;

		skillMoveThisTick = {};
	}
	SH_USER_API void PlayerMovement::TickUpdate(uint64_t tick)
	{
	}
	SH_USER_API void PlayerMovement::ProcessInput(const PlayerInputPacket& packet)
	{
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
		recv.applyServerTick = player->EstimateApplyServerTick(packet.tick);
		recv.state = state;
		recvStates.push_back(recv);
	}
}//namespace
