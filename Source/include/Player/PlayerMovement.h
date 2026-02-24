#pragma once
#include "Export.h"
#include "Player.h"
#include "Physics/FootholdMovement.h"
#include "Packet/PlayerStatePacket.hpp"
#include "Packet/PlayerInputPacket.hpp"

#include "Core/EventSubscriber.h"

#include "Game/Component/Component.h"

#include "Network/PacketEvent.hpp"

#include <deque>
namespace sh::game
{
	class PlayerMovement : public FootholdMovement
	{
		COMPONENT(PlayerMovement, "user")
	public:
		SH_USER_API PlayerMovement(GameObject& owner);

		SH_USER_API void Awake() override;
		SH_USER_API void Start() override;
		SH_USER_API void BeginUpdate() override;
		SH_USER_API void FixedUpdate() override;
		SH_USER_API void Update() override;

		SH_USER_API void LockInput() { bInputLock = true; }
		SH_USER_API void UnlockInput() { bInputLock = false; }

		SH_USER_API auto IsInputLock() const -> bool { return bInputLock; }
		SH_USER_API auto GetPlayer() const -> Player* { return player; }
		SH_USER_API auto IsRight() const -> bool { return bRight; }
		SH_USER_API auto IsProne() const -> bool { return lastInput.bProne; }
	private:
#if SH_SERVER
		void ProcessInput(const PlayerInputPacket& packet);
#else
		void ProcessLocalInput();
		void Reconciliation(const PlayerStatePacket& packet);
		void ProcessRemote(const PlayerStatePacket& packet);
		void InterpolateRemote();
#endif
	private:
		PROPERTY(player)
		Player* player = nullptr;

		uint64_t tick = 0;

		core::EventSubscriber<network::PacketEvent> packetSubscriber;
#if SH_SERVER
		uint32_t sendTick = 0;

		struct InputState
		{
			int xMove = 0;
			uint32_t seq = 0;
			uint64_t recvServerTick = 0; // 받았을 때 당시 서버 물리 틱
			uint64_t clientTick = 0; // 해당 input을 요청 했을 당시 클라 물리 틱
			bool bJump = false;
			bool bProne = false;
		} lastInput;

		bool bSend = false;
#else
		uint64_t nextSeq = 1;
		struct LastInput
		{
			uint32_t seq = 0;
			int xMove = 0;
			bool bJump = false;
			bool bProne = false;
		} lastInput;

		struct StateHistory
		{	
			uint32_t seq = 0;
			uint64_t tick = 0;
			Vec2 pos;
			Vec2 vel;
			int xMove = 0;
			bool bJump = false;
			bool bProne = false;
		};
		std::deque<StateHistory> history;

		Vec2 serverPos{ 0.f, 0.f };
#endif
		bool bRight = false;
		bool bProne = false;
		bool bInputLock = false;
	};
}//namespace