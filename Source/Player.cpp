#include "Player.h"

namespace sh::game
{
	Player::Player(GameObject& owner) :
		NetworkComponent(owner), userUUID(core::UUID::GenerateEmptyUUID())
	{
	}
	SH_USER_API void Player::SetUserUUID(const core::UUID& uuid)
	{
		userUUID = uuid;
	}
	SH_USER_API auto Player::GetUserUUID() const -> const core::UUID&
	{
		return userUUID;
	}
	SH_USER_API void Player::Start()
	{
		SH_INFO("Hello?");
	}
}
