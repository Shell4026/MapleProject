#pragma once
#include "Export.h"
#include "Player.h"
#include "Physics/Foothold.h"
#include "Packet/PlayerStatePacket.hpp"
#include "Packet/PlayerInputPacket.hpp"

#include "Core/EventSubscriber.h"

#include "Game/Component/Component.h"

#include "Network/PacketEvent.hpp"

#include <deque>
namespace sh::game
{
	class PlayerMovement : public Component
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
		SH_USER_API auto GetSpeed() const -> float { return speed; }
		SH_USER_API auto GetJumpSpeed() const -> float { return jumpSpeed; }
		SH_USER_API auto GetVelocity() const -> Vec2 { return Vec2{ velX, velY }; }
		SH_USER_API auto IsGround() const -> bool { return bGround; }
	private:
		void StepMovement();
		void ApplyGravity();
		void ApplyPos();
		void CheckGround();
		void MoveOnGround();
		void ClampPos();
#if SH_SERVER
		void ProcessInput(const PlayerInputPacket& packet);
#else
		void ProcessLocalInput();
		void Reconciliation(const PlayerStatePacket& packet);
#endif
	private:
		constexpr static float G = 20.f;

		PROPERTY(player)
		Player* player = nullptr;

		PROPERTY(speed)
		float speed = 1.25f;
		PROPERTY(jumpSpeed)
		float jumpSpeed = 5.55f;
		PROPERTY(maxFallSpeed)
		float maxFallSpeed = 6.7f;
		PROPERTY(maxStepHeight)
		float maxStepHeight = 0.2f;

		Foothold* foothold = nullptr;
		Foothold::Contact ground;

		float velX = 0.f;
		float velY = 0.f;
		float offset = 0.1f;
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
#endif
		bool bGround = false;
		bool bProne = false;
		bool bInputLock = false;
	};
}//namespace