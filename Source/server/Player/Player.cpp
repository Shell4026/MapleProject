#include "Player/Player.h"
#include "Player/PlayerMovement.h"
#include "World/MapleWorld.h"
#include "skill/SkillManager.h"
#include "Packet/PlayerInputPacket.hpp"
#include "Packet/PlayerStatePacket.hpp"

namespace sh::game
{
	SH_USER_API void Player::BroadcastState()
	{
		auto& bodyPos = movement->gameObject.transform->GetWorldPosition();

		PlayerStatePacket packet{};
		packet.lastProcessedInputSeq = movement->GetAppliedInputSeq();
		packet.serverTick = GetTick();
		packet.clientTickAtState = movement->GetAppliedClientTick() + (GetTick() - movement->GetAppliedServerTick());

		packet.px = bodyPos.x;
		packet.py = bodyPos.y;
		packet.vx = movement->GetVelocity().x;
		packet.vy = movement->GetVelocity().y;
		packet.playerUUID = GetUUID();
		packet.bGround = movement->IsGround();
		packet.bLock = movement->IsInputLock();
		packet.bUp = movement->IsUp();
		packet.bProne = movement->IsProne();
		packet.bRight = movement->IsRight();
		packet.skillId = skillManager->GetUsingSkillId();
		packet.bSkillUsing = skillManager->IsUsingSkill();

		if (currentWorld != nullptr)
			currentWorld->BroadCastToWorld(packet);
		else
			SH_ERROR("currentWorld is nullptr!");
	}
	SH_USER_API void Player::ProcessInputPacket(const PlayerInputPacket& packet)
	{
		CalcTickOffset(packet.tick);
		if (lastProcessedSeq >= packet.seq)
			return;

		lastProcessedSeq = packet.seq;
		movement->ProcessInput(packet);
		bSend = true;
	}
	SH_USER_API void Player::ProcessSkillPacket(const SkillUsingPacket& packet)
	{
		CalcTickOffset(packet.tick);
		if (lastProcessedSeq >= packet.seq)
			return;

		lastProcessedSeq = packet.seq;
		skillManager->ProcessPacket(packet);
		bSend = true;
	}

	void Player::CalcTickOffset(uint64_t clientTick)
	{
		const uint64_t tick = GetTick();
		if (!bOffsetInit)
		{
			tickOffset = tick - clientTick + 5;
			bOffsetInit = true;
		}
		else
		{
			const uint64_t newOffset = tick - clientTick + 5;
			tickOffset = (tickOffset * 9 + newOffset) / 10;
		}
	}
}//namespace
