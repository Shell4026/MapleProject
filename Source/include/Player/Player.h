#pragma once
#include "Export.h"
#if !SH_SERVER
#include "NameTag.h"
#endif

#include "Game/Component/Component.h"

namespace sh::game
{
	class MapleWorld;
	class Player : public Component
	{
		COMPONENT(Player, "user")
	public:
		SH_USER_API Player(GameObject& owner);

		SH_USER_API void SetUserUUID(const core::UUID& uuid);
		SH_USER_API auto GetUserUUID() const -> const core::UUID&;

		SH_USER_API auto IsLocal() const -> bool;

		SH_USER_API void SetRight(bool bRight);
		SH_USER_API auto IsRight() const -> bool;

		SH_USER_API void SetCurrentWorld(MapleWorld& world) { currentWorld = &world; }
		SH_USER_API auto GetCurrentWorld() const -> MapleWorld* { return currentWorld; }

#if !SH_SERVER
		SH_USER_API auto GetNameTag() const -> NameTag* { return nametag; }
#endif
	private:
		core::UUID userUUID;

		MapleWorld* currentWorld = nullptr;
#if !SH_SERVER
		PROPERTY(nametag)
		NameTag* nametag = nullptr;
#endif

		bool bLocal = true;
		bool bRight = false;
	};
}//namespace