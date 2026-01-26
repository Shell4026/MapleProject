#pragma once
#include "Export.h"
#include "UI/UIRect.h"

#include "Core/Observer.hpp"

#include "Render/Texture.h"

#include "Game/Vector.h"
#include "Game/Component/Render/MeshRenderer.h"

#include <vector>
namespace sh::game
{
	class DialogUI : public UIRect
	{
		COMPONENT(DialogUI, "user")
	public:
		SH_USER_API DialogUI(GameObject& owner);

		SH_USER_API void Awake() override;
		SH_USER_API void Update() override;

		SH_USER_API void OnClick() override;

		SH_USER_API void SetNpcTexture(const render::Texture& texture);
	private:
		void Dragging();
	private:
		PROPERTY(npcRenderer)
		MeshRenderer* npcRenderer = nullptr;

		Vec3 lastPos;
		Vec2 clickedPos{0.f, 0.f};

		bool bDragging = false;
	};
}//namespace