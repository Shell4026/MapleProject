﻿#pragma once
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

//#define SH_SERVER 1
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
		SH_USER_API void FixedUpdate() override;
		SH_USER_API void Update() override;
		SH_USER_API void OnCollisionEnter(Collider& collider) override;

		SH_USER_API auto GetPlayer() const -> Player*;

		/// @brief 조작을 막는다.
		SH_USER_API void Lock();
		/// @brief 조작을 허용한다.
		SH_USER_API void Unlock();
		SH_USER_API auto IsLock() const -> bool;
	private:
		
#if SH_SERVER
		void ProcessInputPacket(const PlayerInputPacket& packet, const Endpoint& endpoint);
		/// @brief 이전에 왔던 입력 기반 움직임 적용 함수
		void ProcessInput();
#else
		void ProcessLocalInput();
		void ProcessStatePacket(const PlayerStatePacket& packet);
		void ProcessLocalAnim(float xInput);
		void ProcessRemoteAnim();
#endif
	private:
		PROPERTY(rigidBody)
		RigidBody* rigidBody = nullptr;
#if !SH_SERVER
		PROPERTY(anim)
		PlayerAnimation* anim = nullptr;
#endif
		PROPERTY(player)
		Player* player = nullptr;

		PROPERTY(speed)
		float speed = 1.25f;
		PROPERTY(jumpSpeed)
		float jumpSpeed = 5.55f;
		PROPERTY(maxFallSpeed)
		float maxFallSpeed = 6.7f;

		core::SObjWeakPtr<RigidBody> floor;

		float xVelocity = 0.f;
		float yVelocity = 0.f;

		float rayDistance = 1.0f;
		float floorY = -1000.0f;

		core::EventSubscriber<PacketEvent> packetEventSubscriber;
		
#if !SH_SERVER
		MapleClient* client = nullptr;

		glm::vec2 serverPos;
		glm::vec2 serverVel;
		PlayerInputPacket lastSent;

		uint64_t inputSeqCounter = 1;
		uint64_t tick = 0;

		bool bReceived = false;
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
			bool bJump = false;
			bool bProne = false;
		} lastInput;

		struct LastSent
		{
			glm::vec2 pos;
			glm::vec2 vel;
			uint32_t seq = 0;
		} lastSent;
		bool bSend = false;
#endif
		bool bGround = false;
		bool bProne = false;
		bool bLock = false;
	};
}//namespace