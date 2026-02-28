#include "Player/PlayerTickController.h"
#include "Player/IPlayerTickable.h"

namespace sh::game
{
	SH_USER_API void PlayerTickController::BeginUpdate()
	{
		const uint64_t currentTick = tick;

		for (IPlayerTickable* tickable : tickables)
			tickable->TickBegin(currentTick);
	}
	SH_USER_API void PlayerTickController::FixedUpdate()
	{
		const uint64_t currentTick = tick;

		for (IPlayerTickable* tickable : tickables)
			tickable->TickFixed(currentTick);

		++tick;
	}
	SH_USER_API void PlayerTickController::Update()
	{
		const uint64_t currentTick = tick;

		for (IPlayerTickable* tickable : tickables)
			tickable->TickUpdate(currentTick);
	}
}//namespace
