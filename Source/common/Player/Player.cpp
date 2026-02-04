#include "Player/Player.h"

#if !SH_SERVER
#include "MapleClient.h"
#endif
namespace sh::game
{
	Player::Player(GameObject& owner) :
		Component(owner), userUUID(core::UUID::GenerateEmptyUUID())
	{
	}
	SH_USER_API void Player::Start()
	{
#if !SH_SERVER
		if (bLocal)
		{
			if (inventoryPrefab == nullptr)
				SH_ERROR("Inventory prefab is nullptr");
			else
				inventoryPrefab->AddToWorld(world);
		}
#endif
	}
	SH_USER_API void Player::SetUserUUID(const core::UUID& uuid)
	{
		userUUID = uuid;
#if !SH_SERVER
		bLocal = MapleClient::GetUser().GetUserUUID() == uuid;
#endif
	}
	SH_USER_API auto Player::GetUserUUID() const -> const core::UUID&
	{
		return userUUID;
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
}//namespace