#pragma once
#include "Export.h"
#include "EndPoint.hpp"
#include "PlayerAnimation.h"
#include "Player.h"
#include "MapleServer.h"
#include "MapleClient.h"
#include "PacketEvent.hpp"
#include "Packet/PlayerInputPacket.h"
#include "Packet/PlayerStatePacket.h"

#include "Core/SContainer.hpp"
#include "Core/EventSubscriber.h"

#include "Game/Component/Component.h"
#include "Game/Component/RigidBody.h"

#include <deque>
namespace sh::game
{
	class PlayerMovement2D : public Component
	{
		COMPONENT(PlayerMovement2D, "user")
	public:
		SH_USER_API PlayerMovement2D(GameObject& owner);

		SH_USER_API void Awake() override;
		SH_USER_API void Start() override;
		SH_USER_API void BeginUpdate() override;
		SH_USER_API void Update() override;
	private:
		
#if SH_SERVER
		void ProcessInputPacket(const PlayerInputPacket& packet, const Endpoint& endpoint);
		/// @brief 이전에 왔던 입력 기반 움직임 적용 함수
		void ProcessInput();
#else
		void ProcessLocalInput();
		void ProcessStatePacket(const PlayerStatePacket& packet, bool bLocal);
		void ProcessRemoteAnim();
#endif
	private:
		PROPERTY(rigidBody)
		RigidBody* rigidBody = nullptr;
		PROPERTY(anim)
		PlayerAnimation* anim = nullptr;
		PROPERTY(player)
		Player* player = nullptr;

		PROPERTY(speed)
		float speed = 1.5f;
		PROPERTY(jumpSpeed)
		float jumpSpeed = 5.5f;

		core::SObjWeakPtr<RigidBody> floor;

		float xVelocity = 0.f;
		float yVelocity = 0.f;

		float floorY = -1000.0f;

		core::EventSubscriber<PacketEvent> packetEventSubscriber;
		
#if !SH_SERVER
		MapleClient* client = nullptr;
		std::deque<PlayerInputPacket> pendingInputs;

		uint64_t inputSeqCounter = 0;
		uint64_t tick = 0;
#else
		MapleServer* server = nullptr;
		uint32_t serverTick = 0;

		uint32_t lastProcessedSeq = 0; // 마지막으로 처리한 seq
		uint64_t lastTick = 0; // 마지막으로 처리한 클라이언트측 틱
		struct InputState
		{
			float xMove = 0.f;
			uint32_t seq = 0;
			uint64_t tick = 0;
		} lastInput;

		struct LastSent
		{
			glm::vec2 pos;
			glm::vec2 vel;
			uint32_t seq = 0;
		} lastSent;
#endif
		bool bGround = false;
		bool bJump = false;
	};
}//namespace