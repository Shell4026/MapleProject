#pragma once
#include "Export.h"
#include "UI/UI.h"
#include "UI/InventorySlotUI.h"

#include "Core/Observer.hpp"

#include "Game/Component/MeshRenderer.h"

#include <vector>
namespace sh::game
{
	class InventoryUI : public UI
	{
		COMPONENT(InventoryUI, "user")
	public:
		SH_USER_API InventoryUI(GameObject& owner);

		SH_USER_API void Awake() override;
		SH_USER_API void OnEnable() override;
		SH_USER_API void Update() override;
	private:
		void RenderInventory();
	private:
		PROPERTY(slots)
		std::vector<InventorySlotUI*> slots;

		core::Observer<false, int>::Listener onClickListener;

		int selectedSlotIdx = -1;
	};
}//namespace