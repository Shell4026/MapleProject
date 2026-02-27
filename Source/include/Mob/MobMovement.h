#pragma once
#include "Export.h"
#include "Physics/FootholdMovement.h"

#include "Game/Component/Component.h"

namespace sh::game
{
	class MobMovement : public FootholdMovement
	{
		COMPONENT(MobMovement, "user")
	public:
		SH_USER_API MobMovement(GameObject& owner);

		SH_USER_API void FixedUpdate() override;
	};
}//namespace