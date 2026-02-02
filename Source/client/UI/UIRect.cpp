#include "UI/UIRect.h"
#include "UI/UIInputManager.h"
#include "UI/InventoryUI.h"

#include "Game/World.h"
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
		if (UIInputManager::IsValidInstance())
			UIInputManager::GetInstance().BuildRectList(*this);
		Super::OnDestroy();
	}
	SH_USER_API auto UIRect::IsContainsMouse() const -> bool
	{
		const auto& worldPos = gameObject.transform->GetWorldPosition();

		glm::vec3 screenPos{};
		if (!bWorldPos)
			screenPos = worldPos;
		else
		{
			auto camPtr = world.GetMainCamera();
			const auto camPos = camPtr->GetProjMatrix() * camPtr->GetViewMatrix() * glm::vec4{ worldPos.x, worldPos.y, worldPos.z, 1.0f };
			screenPos = glm::vec3{ (camPos.x * 0.5f + 0.5f) * camPtr->GetWidth(), (camPos.y * 0.5f + 0.5f) * camPtr->GetHeight(), 0.f } * 0.01f;
		}

		const float minX = std::min(screenPos.x + origin.x, screenPos.x + origin.x + size.x);
		const float maxX = std::max(screenPos.x + origin.x, screenPos.x + origin.x + size.x);
		const float minY = std::min(screenPos.y + origin.y, screenPos.y + origin.y + size.y);
		const float maxY = std::max(screenPos.y + origin.y, screenPos.y + origin.y + size.y);

		glm::vec2 mousePos = Input::mousePosition;
		mousePos.y = world.renderer.GetHeight() - mousePos.y;
		mousePos /= 100.f;

		if (minX <= mousePos.x && mousePos.x <= maxX &&
			minY <= mousePos.y && mousePos.y <= maxY)
		{
			return true;
		}
		return false;
	}
}//namespace