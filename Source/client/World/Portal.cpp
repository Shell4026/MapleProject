#include "World/Portal.h"

namespace sh::game
{
	Portal::Portal(GameObject& owner) :
		Component(owner)
	{
	}

	SH_USER_API void Portal::Awake()
	{
	}

	SH_USER_API void Portal::OnDestroy()
	{
		Super::OnDestroy();
	}

	SH_USER_API void Portal::OnTriggerEnter(Collider& collider)
	{
	}

	SH_USER_API void Portal::OnTriggerExit(Collider& collider)
	{
	}
}//namespace
