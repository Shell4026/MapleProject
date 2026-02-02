#include "UI/DialogUI.h"
#include "MapleClient.h"

#include "Game/World.h"
#include "Game/Input.h"

#include <queue>
namespace sh::game
{
	DialogUI::DialogUI(GameObject& owner) :
		UIRect(owner)
	{
		onClickCloseButton.SetCallback(
			[this](UIRect*)
			{
				gameObject.Destroy();
				bOpen = false;
			}
		);
	}
	SH_USER_API void DialogUI::Awake()
	{
		Super::Awake();
		if (closeButton != nullptr)
			closeButton->onClick.Register(onClickCloseButton);
		if (textRenderer != nullptr)
			textRenderer->SetText("");
		bOpen = true;
	}
	SH_USER_API void DialogUI::Update()
	{
		TextAnimation();
		Dragging();
	}
	SH_USER_API void DialogUI::OnHover()
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
		{
			npcRenderer->GetMaterialPropertyBlock()->SetProperty("tex", &texture);
			npcRenderer->UpdatePropertyBlockData();
		}
	}
	SH_USER_API void DialogUI::SetText(const std::string& text)
	{
		this->text = text;
	}
	SH_USER_API auto DialogUI::IsOpen() -> bool
	{
		return bOpen;
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
	void DialogUI::TextAnimation()
	{
		if (textRenderer == nullptr)
			return;

		if (textCursor == text.size())
			return;

		textTime += world.deltaTime;
		if (textTime < textAnimationTime)
			return;

		textTime = 0.f;
		++textCursor;
		std::string_view txt{ text.data(), static_cast<std::size_t>(textCursor) };
		textRenderer->SetText(std::string{ txt });
	}
}//namespace