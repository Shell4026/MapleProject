#pragma once
#include "Export.h"
#include "MobEvents.hpp"

#include "Game/Component/Component.h"
#include "Game/Component/Sound/AudioSource.h"

#include "Core/EventSubscriber.h"

#include "Sound/SoundClip.h"

namespace sh::game
{
	class Mob;
	class MobSoundController : public Component
	{
		COMPONENT(MobSoundController, "user")
	public:
		SH_USER_API MobSoundController(GameObject& owner);

		SH_USER_API void Awake() override;

		SH_USER_API auto GetMob() const -> Mob& { return *mob; }
		SH_USER_API auto GetAudioSource() const -> AudioSource& { return *audioSource; }
	private:
		PROPERTY(mob, core::PropertyOption::sobjPtr)
		Mob* mob = nullptr;
		PROPERTY(audioSource)
		AudioSource* audioSource = nullptr;

		core::EventSubscriber<MobDeathEvent> onMobDeath;
	};
}//namespace