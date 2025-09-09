#pragma once
#include "Export.h"
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
		void Jump();
		void ProcessStatePacket(const PlayerStatePacket& packet);
		void ProcessInputPacket(const PlayerInputPacket& packet);
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
		uint32_t nextInputSeq = 1;
		std::deque<PlayerInputPacket> pendingInputs;
#else
		MapleServer* server = nullptr;
		uint32_t lastProcessedSeq = 0;
		uint32_t serverTick = 0;
#endif
		bool bGround = false;
	};
}//namespace