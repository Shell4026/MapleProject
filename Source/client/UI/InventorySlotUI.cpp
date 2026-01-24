#include "UI/InventorySlotUI.h"

#include "Core/Logger.h"

#include "Game/GameObject.h"
#include "Game/Input.h"
#include "Game/Component/Camera.h"

namespace sh::game
{
	InventorySlotUI::InventorySlotUI(GameObject& owner) :
		UIRect(owner)
	{
	}
	SH_USER_API void InventorySlotUI::BeginUpdate()
	{
		// Inventory에서 처리하게
	}
	SH_USER_API void InventorySlotUI::OnClick()
	{
		if (Input::GetMouseReleased(Input::MouseType::Left))
		{
			SH_INFO("click!");
			onClick.Notify(idx);
		}
	}
}//namespace