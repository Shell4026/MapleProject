#include "UI/InventorySlotUI.h"

#include "Core/Logger.h"

#include "Game/GameObject.h"
#include "Game/Input.h"

namespace sh::game
{
	InventorySlotUI::InventorySlotUI(GameObject& owner) :
		UIRect(owner)
	{
	}
	SH_USER_API void InventorySlotUI::OnHover()
	{
		if (Input::GetMouseReleased(Input::MouseType::Left))
		{
			onClick.Notify(this);
		}
	}
}//namespace