#include "World/Portal.h"

#include "Phys/CollisionTag.hpp"
#include "World/MapleServer.h"
#include "World/MapleWorld.h"
#include "Player/Player.h"
#include "Player/PlayerHitbox.h"

#include "Game/Component/Phys/Collider.h"
#include "Game/GameObject.h"
#include "Game/World.h"

// 서버 사이드
namespace sh::game
{
	namespace
	{
		constexpr float portalTransferCooldownMs = 500.f;
	}

	Portal::Portal(GameObject& owner) :
		Component(owner)
	{
	}

	SH_USER_API void Portal::Awake()
	{
		if (Collider* collider = gameObject.GetComponent<Collider>(true); collider != nullptr)
		{
			collider->SetTrigger(true);
			collider->SetCollisionTag(tag::portalTag);
			collider->SetAllowCollisions(tag::playerTag);
			SH_INFO("awake");
		}

		mapleWorld = MapleWorld::GetMapleWorld(world.GetUUID());
		if (mapleWorld == nullptr)
		{
			SH_ERROR_FORMAT("MapleWorld is not found for portal: {}", world.GetUUID().ToString());
			return;
		}

		mapleWorld->RegisterPortal(*this);
	}

	SH_USER_API void Portal::OnDestroy()
	{
		if (mapleWorld != nullptr)
			mapleWorld->UnRegisterPortal(this);
		Super::OnDestroy();
	}

	SH_USER_API void Portal::OnTriggerEnter(Collider& collider)
	{
		if (collider.GetCollisionTag() != tag::playerTag)
			return;
		Player* const player = static_cast<PlayerHitbox&>(collider).GetPlayer();

		if (player == nullptr)
			return;

		playersInPortal.insert(player->GetUserUUID());
	}

	SH_USER_API void Portal::OnTriggerExit(Collider& collider)
	{
		if (collider.GetCollisionTag() != tag::playerTag)
			return;

		Player* const player = static_cast<PlayerHitbox&>(collider).GetPlayer();

		if (player == nullptr)
			return;

		playersInPortal.erase(player->GetUserUUID());
	}

	SH_USER_API auto Portal::TryTransfer(Player& player) -> bool
	{
		if (targetWorld == nullptr)
			return false;

		if (playersInPortal.find(player.GetUserUUID()) == playersInPortal.end())
			return false;

		MapleServer* const server = MapleServer::GetInstance();
		if (server == nullptr)
			return false;

		User* const user = server->GetUserManager().GetUser(player.GetUserUUID());
		if (user == nullptr || !user->CanTransferPortal())
			return false;

		const bool bMoved = server->MoveUserToWorld(player.GetUserUUID(), targetWorld->GetUUID(), portalId);
		if (bMoved)
			user->SetPortalTransferCooldownMs(portalTransferCooldownMs);

		return bMoved;
	}
}//namespace
