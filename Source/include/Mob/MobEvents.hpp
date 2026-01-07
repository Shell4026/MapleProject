#pragma once
#include "Core/IEvent.h"

#include "Game/WorldEvents.hpp"

namespace sh::game
{
	class Mob;

	struct MobDeathEvent : core::IEvent
	{
		Mob& mob;

		MobDeathEvent(Mob& mob) :
			mob(mob)
		{
		}
		auto GetTypeHash() const -> std::size_t override
		{
			return core::reflection::TypeTraits::GetTypeHash<MobDeathEvent>();
		}
	};
}//namespace