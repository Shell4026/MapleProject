#pragma once
#include "Export.h"

#include "Core/UUID.h"

#include "Game/Component/Component.h"

#include <unordered_set>
namespace sh::game
{
	class Player;
	class MapleWorld;
	class World;
	class Portal : public Component
	{
		COMPONENT(Portal, "user")
	public:
		SH_USER_API Portal(GameObject& owner);

		SH_USER_API void Awake() override;
		SH_USER_API void OnDestroy() override;

		SH_USER_API void OnTriggerEnter(Collider& collider) override;
		SH_USER_API void OnTriggerExit(Collider& collider) override;

#if SH_SERVER
		SH_USER_API auto TryTransfer(Player& player) -> bool;
#endif

		SH_USER_API auto GetPortalId() const -> int { return portalId; }
	private:
		PROPERTY(portalId)
		int portalId = 0;
		PROPERTY(targetWorld, core::PropertyOption::sobjPtr)
		World* targetWorld = nullptr;

#if SH_SERVER
		MapleWorld* mapleWorld = nullptr;
		std::unordered_set<core::UUID> playersInPortal;
#endif
	};
}//namespace
