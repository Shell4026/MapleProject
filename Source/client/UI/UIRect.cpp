#include "UI/UIRect.h"

#include "Game/GameObject.h"
#include "Game/Input.h"
#include "Game/Component/Camera.h"

#include <stack>
namespace sh::game
{
	UIRect::UIRect(GameObject& owner) :
		UI(owner)
	{
	}
	SH_USER_API auto UIRect::CheckMouseHit() const -> bool
	{
		const auto& pos = gameObject.transform->GetWorldPosition();
		const float minX = std::min(pos.x + origin.x, pos.x + origin.x + size.x);
		const float maxX = std::max(pos.x + origin.x, pos.x + origin.x + size.x);
		const float minY = std::min(pos.y + origin.y, pos.y + origin.y + size.y);
		const float maxY = std::max(pos.y + origin.y, pos.y + origin.y + size.y);

		auto camera = world.GetMainCamera();
		if (camera == nullptr)
		{
			SH_ERROR("Main camera is nullptr!");
			return false;
		}
		glm::vec2 worldMousePos = Input::mousePosition;
		worldMousePos.y = world.renderer.GetHeight() - worldMousePos.y;
		worldMousePos /= 100.f;

		//SH_INFO_FORMAT("mouseX: {}, mouseY: {}", worldMousePos.x, worldMousePos.y);
		//SH_INFO_FORMAT("minY: {}, maxY: {}", minY, maxY);
		if (minX <= worldMousePos.x && worldMousePos.x <= maxX &&
			minY <= worldMousePos.y && worldMousePos.y <= maxY)
		{
			return true;
		}
		return false;
	}
}//namespace