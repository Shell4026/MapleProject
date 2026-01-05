#pragma once
#include "Export.h"
#include "Player/Player.h"

#include "Game/Component/Component.h"

namespace sh::game
{
	class Mob;
	class AIStrategy : public Component
	{
		SCLASS(AIStrategy)
	public:
		SH_USER_API AIStrategy(GameObject& owner);

		virtual void Run(Mob& mob) {}
		virtual void OnAttacked(Player& player) {}
		virtual auto GetState() const -> uint32_t { return 0; }
		virtual void Reset() = 0;
	};
}//namespace