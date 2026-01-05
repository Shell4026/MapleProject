#pragma once
#include "Export.h"

#include "Game/Component/NetworkComponent.h"

namespace sh::game
{
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
	private:
		core::UUID userUUID;

		uint32_t heartbeat;

		bool bLocal = true;
		bool bRight = false;
	};
}