#pragma once
#include "Export.h"

#include "Game/Component/Component.h"

#include <cstdint>

namespace sh::game
{
	class Player;
	class PlayerMovement;
	class SkillManager;
	class PlayerTickController : public Component
	{
		COMPONENT(PlayerTickController, "user")
	public:
		SH_USER_API PlayerTickController(GameObject& owner);

		SH_USER_API void Awake() override;
		SH_USER_API void BeginUpdate() override;
		SH_USER_API void FixedUpdate() override;
		SH_USER_API void Update() override;

		SH_USER_API auto GetTick() const -> uint64_t { return tick; }
	private:
		PROPERTY(player, core::PropertyOption::sobjPtr)
		Player* player = nullptr;
		uint64_t tick = 0;
	};
}//namespace
