#pragma once
#include "Export.h"
#include "UI/UIRect.h"
#include "UI/Button.h"

#include "Core/Observer.hpp"

#include "Render/Texture.h"

#include "Game/Vector.h"
#include "Game/Component/Render/MeshRenderer.h"
#include "Game/Component/Render/TextRenderer.h"

#include <vector>
#include <string>
namespace sh::game
{
	class DialogUI : public UIRect
	{
		COMPONENT(DialogUI, "user")
	public:
		SH_USER_API DialogUI(GameObject& owner);

		SH_USER_API void Awake() override;
		SH_USER_API void Update() override;

		SH_USER_API void OnHover() override;

		SH_USER_API void SetNpcTexture(const render::Texture& texture);
		SH_USER_API void SetText(const std::string& text);

		SH_USER_API auto IsValid() const -> bool { return npcRenderer != nullptr && textRenderer != nullptr; }

		SH_USER_API static auto IsOpen() -> bool;
	private:
		void Dragging();
		void TextAnimation();
	private:
		PROPERTY(npcRenderer)
		MeshRenderer* npcRenderer = nullptr;
		PROPERTY(textRenderer)
		TextRenderer* textRenderer = nullptr;
		PROPERTY(closeButton)
		Button* closeButton = nullptr;
		PROPERTY(textAnimationTime)
		float textAnimationTime = 0.01f;

		Vec3 lastPos;
		Vec2 clickedPos{0.f, 0.f};

		core::Observer<false, UIRect*>::Listener onClickCloseButton;

		std::string text;

		int textCursor = 0;
		float textTime = 0.f;

		bool bDragging = false;
		inline static bool bOpen = false;
	};
}//namespace