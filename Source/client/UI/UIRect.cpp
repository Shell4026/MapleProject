#include "UI/UIRect.h"

#include "Game/GameObject.h"
#include "Game/Input.h"
#include "Game/Component/Render/Camera.h"

#include <stack>
namespace sh::game
{
	UIRect::UIRect(GameObject& owner) :
		UI(owner)
	{
	}
	SH_USER_API void UIRect::BeginUpdate()
	{
		HitTest();
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
	void UIRect::HitTest()
	{
		if (!CheckMouseHit())
			return;

		std::queue<Transform*> bfs;

		bfs.push(gameObject.transform);
		UIRect* lastRect = nullptr;
		while (!bfs.empty())
		{
			Transform* cur = bfs.front();
			bfs.pop();
			GameObject& obj = cur->gameObject;

			bool bHitSuccess = true;
			for (auto component : obj.GetComponents())
			{
				UIRect* rect = core::reflection::Cast<UIRect>(component);
				if (rect != nullptr)
				{
					bHitSuccess = rect->CheckMouseHit();
					if (bHitSuccess)
					{
						lastRect = rect;
						break;
					}
				}
			}
			if (!bHitSuccess)
				continue;
			for (auto child : cur->GetChildren())
				bfs.push(child);
		}

		if (lastRect == nullptr)
			OnClick();
		else
			lastRect->OnClick();
	}
}//namespace