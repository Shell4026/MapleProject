#pragma once
#include "Export.h"
#include "IPlayerTickable.h"

#include <cstdint>
#include <vector>
namespace sh::game
{
	class PlayerTickController
	{
	public:
		SH_USER_API void BeginUpdate();
		SH_USER_API void FixedUpdate();
		SH_USER_API void Update();

		SH_USER_API void RegisterTickable(IPlayerTickable* tickable) { tickables.push_back(tickable); }

		SH_USER_API auto GetTick() const -> uint64_t { return tick; }
	private:
		uint64_t tick = 0;
		std::vector<IPlayerTickable*> tickables;
	};
}//namespace
