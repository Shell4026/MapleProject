#pragma once
#include "Export.h"
#include "Game/Component/Component.h"
#include "Game/Component/Sound/AudioSource.h"
#include "Sound/SoundClip.h"

namespace sh::game
{
	class Player;
	class PlayerSoundController : public Component
	{
		COMPONENT(PlayerSoundController, "user")
	public:
		SH_USER_API PlayerSoundController(GameObject& owner);

		SH_USER_API void Awake() override;
		SH_USER_API void FixedUpdate() override;

		SH_USER_API auto GetPlayer() const -> Player& { return *player; }
		SH_USER_API auto GetAudioSource() const -> AudioSource& { return *audioSource; }
		SH_USER_API auto GetJumpSound() const -> sound::SoundClip* { return jumpSound; }
	private:
		PROPERTY(player, core::PropertyOption::sobjPtr)
		Player* player = nullptr;
		PROPERTY(audioSource)
		AudioSource* audioSource = nullptr;
		PROPERTY(jumpSound)
		sound::SoundClip* jumpSound = nullptr;
	};
}//namespace