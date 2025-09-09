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
		void ProcessStatePacket(const PlayerStatePacket& packet);
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

		uint64_t tick = 0;
		struct LastSent
		{
			float xMove = 0.f;
			uint32_t inputSeqCounter = 0;
			bool bJump = false;
		} lastSent;
#else
		MapleServer* server = nullptr;
		uint32_t serverTick = 0;

		struct PlayerData
		{
			uint32_t lastProcessedSeq = 0; // 마지막으로 처리한 seq
			struct InputState
			{
				float xMove = 0.f;
				uint32_t seq = 0;
				bool jump = false;
			}lastInput;
			struct LastSent
			{
				glm::vec2 pos;
				glm::vec2 vel;
				uint32_t seq = 0;
				bool bFirst = true;
			} lastSent;
		} playerData;
#endif
		bool bGround = false;
	};
}//namespace