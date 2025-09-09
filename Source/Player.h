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

		SH_USER_API void Start() override;
	private:
		core::UUID userUUID;
		bool bLocal = true;
	};
}