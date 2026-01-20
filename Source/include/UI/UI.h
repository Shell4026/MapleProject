#pragma once
#include "Export.h"

#include "Game/Component/Component.h"

namespace sh::game
{
	class UI : public Component
	{
		SCLASS(UI)
	public:
		SH_USER_API UI(GameObject& owner);
	};
}//namespace