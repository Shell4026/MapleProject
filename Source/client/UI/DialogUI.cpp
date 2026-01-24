#include "UI/DialogUI.h"
#include "MapleClient.h"

#include "Game/GameObject.h"
#include "Game/Input.h"

#include <queue>
namespace sh::game
{
	DialogUI::DialogUI(GameObject& owner) :
		UIRect(owner)
	{
	}
	SH_USER_API void DialogUI::Awake()
	{
	}
	SH_USER_API void DialogUI::Update()
	{
		Dragging();
	}
	SH_USER_API void DialogUI::OnClick()
	{
		if (!bDragging && Input::GetMousePressed(Input::MouseType::Left))
		{
			clickedPos.x = Input::mousePosition.x;
			clickedPos.y = Input::mousePosition.y;
			lastPos = gameObject.transform->GetWorldPosition();
			bDragging = true;
		}
		if (bDragging && Input::GetMouseReleased(Input::MouseType::Left))
		{
			clickedPos.x = Input::mousePosition.x;
			clickedPos.y = Input::mousePosition.y;
			bDragging = false;
		}
	}
	SH_USER_API void DialogUI::SetNpcTexture(const render::Texture& texture)
	{
		if (npcRenderer != nullptr)
			npcRenderer->GetMaterialPropertyBlock()->SetProperty("tex", &texture);
	}
	void DialogUI::Dragging()
	{
		if (!bDragging)
			return;

		glm::vec2 delta = Input::mousePosition - glm::vec2{ clickedPos };
		delta *= 0.01f;
		auto pos = gameObject.transform->GetWorldPosition();
		pos.x = lastPos.x + delta.x;
		pos.y = lastPos.y - delta.y; // 화면 좌표와 월드 좌표가 반전임
		gameObject.transform->SetWorldPosition(pos);
	}
}//namespace