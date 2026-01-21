#pragma once
#include "Export.h"
#include "UI/UIRect.h"
#include "UI/InventorySlotUI.h"

#include "Core/Observer.hpp"

#include "Game/Vector.h"

#include <vector>
namespace sh::game
{
	class InventoryUI : public UIRect
	{
		COMPONENT(InventoryUI, "user")
	public:
		SH_USER_API InventoryUI(GameObject& owner);

		SH_USER_API void Awake() override;
		SH_USER_API void OnEnable() override;
		SH_USER_API void BeginUpdate() override;
		SH_USER_API void Update() override;

		SH_USER_API void OnClick() override;
	private:
		void HitTest();
		void RenderInventory();
		void Dragging();
		void RenderDropWindow();
	private:
		PROPERTY(slots)
		std::vector<InventorySlotUI*> slots;

		core::Observer<false, int>::Listener onClickListener;

		Vec3 lastPos;
		Vec2 clickedPos{0.f, 0.f};

		int selectedSlotIdx = -1;

		bool bDragging = false;
		bool bShowDropWindow = false;
	};
}//namespace