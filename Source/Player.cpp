#include "Player.h"
#include "MapleClient.h"
#include "Packet/HeartbeatPacket.hpp"

#include "Game/World.h"

namespace sh::game
{
	Player::Player(GameObject& owner) :
		NetworkComponent(owner), userUUID(core::UUID::GenerateEmptyUUID())
	{
		heartbeat = 5;
	}
	SH_USER_API void Player::SetUserUUID(const core::UUID& uuid)
	{
		userUUID = uuid;
	}
	SH_USER_API auto Player::GetUserUUID() const -> const core::UUID&
	{
		return userUUID;
	}
	SH_USER_API void Player::IncreaseHeartbeat()
	{
		heartbeat = 5;
	}
	SH_USER_API auto Player::GetHeartbeat() const -> uint32_t
	{
		return heartbeat;
	}
	SH_USER_API auto Player::IsLocal() const -> bool
	{
		return bLocal;
	}
	SH_USER_API void Player::SetRight(bool bRight)
	{
		this->bRight = bRight;
	}
	SH_USER_API auto Player::IsRight() const -> bool
	{
		return bRight;
	}
	SH_USER_API void Player::Awake()
	{
#if !SH_SERVER
		if (userUUID == MapleClient::GetInstance()->GetUser().GetUserUUID())
			bLocal = true;
		else
			bLocal = false;
#endif
	}
	SH_USER_API void Player::Start()
	{
	}
	SH_USER_API void Player::Update()
	{
#if SH_SERVER
		static float t = 0.f;
		t += world.deltaTime;
		if (t >= 1.f)
		{
			heartbeat--;
			t = 0.f;
		}
#else
		if (bLocal)
		{
			static float t = 0.f;
			t += world.deltaTime;
			if (t >= 1.f)
			{
				HeartbeatPacket packet{};
				MapleClient::GetInstance()->SendPacket(packet);
				t = 0.f;
			}
		}
#endif
	}
}
