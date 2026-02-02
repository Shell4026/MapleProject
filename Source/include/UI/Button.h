#pragma once
#include "Export.h"
#include "UI/UIRect.h"

namespace sh::game
{
	class Button : public UIRect
	{
		COMPONENT(Button, "user")
	public:
		SH_USER_API Button(GameObject& owner);

		SH_USER_API void OnHover() override;
	};
}//namespace