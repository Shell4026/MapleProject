#pragma once
#include "Export.h"
#include "Player/IPlayerTickable.h"
#include "Player.h"
#include "Phys/FootholdMovement.h"
#include "Packet/PlayerStatePacket.hpp"
#include "Packet/PlayerInputPacket.hpp"

#include "Core/EventSubscriber.h"

#include "Game/Component/Component.h"

#include "Network/PacketEvent.hpp"

#include <deque>
namespace sh::game
{
	class PlayerMovement : public FootholdMovement, public IPlayerTickable
	{
		COMPONENT(PlayerMovement, "user")
	public:
		SH_USER_API PlayerMovement(GameObject& owner);

		SH_USER_API void Awake() override;
		SH_USER_API void Start() override;

		SH_USER_API void TickBegin(uint64_t tick) override;
		SH_USER_API void TickFixed(uint64_t tick) override;
		SH_USER_API void TickUpdate(uint64_t tick) override;

#if SH_SERVER
		SH_USER_API void ProcessInput(const PlayerInputPacket& packet);
		SH_USER_API auto EstimateApplyServerTick(uint64_t clientTick) const -> uint64_t;
#endif

		SH_USER_API void LockInput() { state.bLock = true; }
		SH_USER_API void UnlockInput() { state.bLock = false; }
		SH_USER_API void SetFacing(bool bRight) { this->bRight = bRight; }
		SH_USER_API void ApplySkillImpulse(float vx, float vy);
		SH_USER_API void ApplySkillTeleport(float dx, float dy);

		SH_USER_API auto GetPlayer() const -> Player* { return player; }
		SH_USER_API auto IsInputLock() const -> bool { return state.bLock; }
		SH_USER_API auto IsRight() const -> bool { return bRight; }
		SH_USER_API auto IsJump() const -> bool { return state.bJump; }
		SH_USER_API auto IsProne() const -> bool { return state.bProne; }
		SH_USER_API auto IsUp() const -> bool { return state.bUp; }
		SH_USER_API auto IsJumpTriggered() const -> bool { return bJumpTriggeredThisTick; }
		SH_USER_API auto IsLandedThisTick() const -> bool { return bLandedThisTick; }
	private:
#if SH_SERVER
#else
		void ProcessLocalInput(uint64_t tick);
		void Reconciliation(const PlayerStatePacket& packet);
		void ProcessRemote(const PlayerStatePacket& packet);
		void InterpolateRemote();
#endif
	private:
		PROPERTY(player)
		Player* player = nullptr;

		struct State
		{
			uint64_t seq = 0;
			int xMove = 0;
			bool bJump = false;
			bool bUp = false;
			bool bProne = false;
			bool bLock = false;
		} state;

		struct SkillMoveHistory
		{
			Vec2 impulse{ 0.f, 0.f };
			Vec2 teleportDelta{ 0.f, 0.f };
			bool bImpulse = false;
			bool bTeleport = false;
			bool bSetExpectedGround = false;
		} skillMoveThisTick;
#if SH_SERVER
		uint32_t sendTick = 0;
		struct RecvState
		{
			uint64_t applyServerTick = 0; // 미래에 적용 할 서버 물리 틱
			uint64_t clientTick = 0; // 해당 input을 요청 했을 당시 클라 물리 틱
			State state;
		};
		std::deque<RecvState> recvStates;
		uint64_t applyServerTick = 0;
		uint64_t clientTick = 0;
		uint64_t offset = 0;
		bool bOffsetInit = false;
#else
		core::EventSubscriber<network::PacketEvent> packetSubscriber;

		uint64_t curSeq = 0;
		uint64_t nextSeq = 1;

		/// @brief seq입력에 대한 결과물
		struct StateHistory
		{
			uint64_t tick = 0;
			Vec2 pos;
			Vec2 vel;
			State state;
			SkillMoveHistory skillMove;
		};
		std::deque<StateHistory> history;

		Vec2 serverPos{ 0.f, 0.f };
#endif
		bool bPendingSend = false;
		bool bRight = false;
		bool bJumpTriggeredThisTick = false;
		bool bLandedThisTick = false;
	};
}//namespace
