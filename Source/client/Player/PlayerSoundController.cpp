#include "Player/PlayerSoundController.h"
#include "Player/PlayerMovement.h"

namespace sh::game
{
	PlayerSoundController::PlayerSoundController(GameObject& owner) :
		Component(owner)
	{
	}
	SH_USER_API void PlayerSoundController::Awake()
	{
		if (player == nullptr)
			SH_ERROR("player is nullptr!");
		if (audioSource == nullptr)
			SH_ERROR("audioSource is nullptr!");

		if (jumpSound == nullptr)
			SH_ERROR("jumpSound is nullptr!");
	}
	SH_USER_API void PlayerSoundController::FixedUpdate()
	{
		if (player->IsLocal())
		{
			if (jumpSound != nullptr && player->GetMovement()->IsJumpTriggered())
			{
				SH_INFO("jump sound");
				audioSource->PlayOneshot(*jumpSound);
			}
		}
	}
}//namespace