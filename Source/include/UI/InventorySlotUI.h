#pragma once
#include "Export.h"
#include "UI/UIRect.h"
#include "UI/NumberUI.h"

#include "Core/Observer.hpp"

#include "Game/Component/Component.h"
#include "Game/Component/Render/MeshRenderer.h"

#include <string>
namespace sh::game
{
	class InventorySlotUI : public UIRect
	{
		COMPONENT(InventorySlotUI, "user")
	public:
		SH_USER_API InventorySlotUI(GameObject& owner);

		SH_USER_API void OnHover() override;

		SH_USER_API void SetIndex(int idx) { this->idx = idx; }
\
		SH_USER_API auto GetRenderer() const -> MeshRenderer* { return renderer; }
		SH_USER_API auto GetNumberUI() const -> NumberUI* { return numberUI; }
	public:
		core::Observer<false, int> onClick;
	private:
		int idx = 0;
		PROPERTY(renderer)
		MeshRenderer* renderer = nullptr;
		PROPERTY(numberUI)
		NumberUI* numberUI = nullptr;
	};
}//namespace