#include "Player/PlayerTickController.h"
#include "Player/IPlayerTickable.h"
#include "Player/Player.h"
#include "Player/PlayerMovement.h"
#include "Skill/SkillManager.h"

#include "Game/GameObject.h"

namespace sh::game
{
	PlayerTickController::PlayerTickController(GameObject& owner) :
		Component(owner)
	{
	}
	SH_USER_API void PlayerTickController::Awake()
	{
		if (player == nullptr)
			player = gameObject.GetComponent<Player>();

		if (player == nullptr)
			SH_ERROR("player is nullptr!");
	}
	SH_USER_API void PlayerTickController::BeginUpdate()
	{
		const uint64_t currentTick = tick;

		if (IPlayerTickable* skillManager = player->GetSkillManager(); skillManager != nullptr)
			skillManager->TickBegin(currentTick);

		if (IPlayerTickable* movement = player->GetMovement(); movement != nullptr)
			movement->TickBegin(currentTick);
	}
	SH_USER_API void PlayerTickController::FixedUpdate()
	{
		const uint64_t currentTick = tick;
		if (IPlayerTickable* skillManager = player->GetSkillManager(); skillManager != nullptr)
			skillManager->TickFixed(currentTick);

		if (IPlayerTickable* movement = player->GetMovement(); movement != nullptr)
			movement->TickFixed(currentTick);

		++tick;
	}
	SH_USER_API void PlayerTickController::Update()
	{
		const uint64_t currentTick = tick;
		if (IPlayerTickable* movement = player->GetMovement(); movement != nullptr)
			movement->TickUpdate(currentTick);
	}
}//namespace
