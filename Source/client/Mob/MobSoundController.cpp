#include "Mob/MobSoundController.h"
#include "Mob/Mob.h"

namespace sh::game
{
	MobSoundController::MobSoundController(GameObject& owner) :
		Component(owner)
	{
		onMobDeath.SetCallback(
			[this](const MobDeathEvent& evt)
			{
				if (mob != nullptr && mob->GetMobData()->GetDieSound() != nullptr)
					audioSource->PlayOneshot(*mob->GetMobData()->GetDieSound());
			}
		);
	}
	SH_USER_API void MobSoundController::Awake()
	{
		if (mob == nullptr)
			SH_ERROR("mob is nullptr!");
		else
			mob->evtBus.Subscribe(onMobDeath);

		if (audioSource == nullptr)
			SH_ERROR("audioSource is nullptr!");
	}
}//namespace