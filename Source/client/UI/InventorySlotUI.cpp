#include "UI/InventorySlotUI.h"

#include "Core/Logger.h"

#include "Game/GameObject.h"
#include "Game/Input.h"
#include "Game/Component/Camera.h"

namespace sh::game
{
	InventorySlotUI::InventorySlotUI(GameObject& owner) :
		Component(owner)
	{
	}
	SH_USER_API void InventorySlotUI::BeginUpdate()
	{
		CheckClick();
	}
	void InventorySlotUI::CheckClick()
	{
		const auto& pos = gameObject.transform->GetWorldPosition();
		const auto& scale = gameObject.transform->scale;
		const float minX = pos.x - scale.x * 0.5f;
		const float maxX = pos.x + scale.x * 0.5f;
		const float minY = pos.y - scale.y * 0.5f;
		const float maxY = pos.y + scale.y * 0.5f;

		if (Input::GetMouseReleased(Input::MouseType::Left))
		{
			auto camera = world.GetMainCamera();
			const auto worldMousePos = camera->ScreenPointToRayOrtho(Input::mousePosition).origin;
			if (minX <= worldMousePos.x && worldMousePos.x <= maxX &&
				minY <= worldMousePos.y && worldMousePos.y <= maxY)
			{
				onClick.Notify(idx);
			}
		}
	}
}//namespace