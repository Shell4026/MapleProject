#pragma once
#include "Export.h"

#include "Game/Component/NetworkComponent.h"

namespace sh::game
{
	class MapleWorld;
	class Player : public NetworkComponent
	{
		COMPONENT(Player, "user")
	public:
		SH_USER_API Player(GameObject& owner);

		SH_USER_API void SetUserUUID(const core::UUID& uuid);
		SH_USER_API auto GetUserUUID() const -> const core::UUID&;

		SH_USER_API void IncreaseHeartbeat();
		SH_USER_API auto GetHeartbeat() const -> uint32_t;

		SH_USER_API auto IsLocal() const -> bool;

		SH_USER_API void SetRight(bool bRight);
		SH_USER_API auto IsRight() const -> bool;

		SH_USER_API void Awake() override;
		SH_USER_API void Start() override;
		SH_USER_API void Update() override;

		SH_USER_API void SetCurrentWorld(MapleWorld& world) { currentWorld = &world; }
		SH_USER_API auto GetCurrentWorld() const -> MapleWorld* { return currentWorld; }
	private:
		core::UUID userUUID;

		uint32_t heartbeat;

		MapleWorld* currentWorld = nullptr;

		bool bLocal = true;
		bool bRight = false;
	};
}