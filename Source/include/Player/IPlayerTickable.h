#pragma once

#include <cstdint>

namespace sh::game
{
	class IPlayerTickable
	{
	public:
		virtual ~IPlayerTickable() = default;

		virtual void TickBegin(uint64_t tick) {}
		virtual void TickFixed(uint64_t tick) {}
		virtual void TickUpdate(uint64_t tick) {}
	};
}//namespace
