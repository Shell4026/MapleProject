#include "UI/UIRect.h"
#include "UI/UIInputManager.h"

#include "Game/GameObject.h"
#include "Game/Input.h"
namespace sh::game
{
	UIRect::UIRect(GameObject& owner) :
		UI(owner)
	{
	}
	SH_USER_API void UIRect::Awake()
	{
		UIInputManager::GetInstance().BuildRectList(*this);
	}
	SH_USER_API void UIRect::OnDestroy()
	{
		UIInputManager::GetInstance().BuildRectList(*this);
		Super::OnDestroy();
	}
	SH_USER_API auto UIRect::IsContainsMouse() const -> bool
	{
		const auto& pos = gameObject.transform->GetWorldPosition();
		const float minX = std::min(pos.x + origin.x, pos.x + origin.x + size.x);
		const float maxX = std::max(pos.x + origin.x, pos.x + origin.x + size.x);
		const float minY = std::min(pos.y + origin.y, pos.y + origin.y + size.y);
		const float maxY = std::max(pos.y + origin.y, pos.y + origin.y + size.y);

		glm::vec2 worldMousePos = Input::mousePosition;
		worldMousePos.y = world.renderer.GetHeight() - worldMousePos.y;
		worldMousePos /= 100.f;

		if (minX <= worldMousePos.x && worldMousePos.x <= maxX &&
			minY <= worldMousePos.y && worldMousePos.y <= maxY)
		{
			return true;
		}
		return false;
	}
}//namespace