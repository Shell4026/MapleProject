#include "UI/InventoryUI.h"
#include "MapleClient.h"
#include "Item/ItemDB.h"
#include "Packet/InventorySlotSwapPacket.hpp"

#include "Game/GameObject.h"
#include "Game/Input.h"
#include "Game/ImGUImpl.h"
#include "Game/Component/Camera.h"

#include <queue>
namespace sh::game
{
	InventoryUI::InventoryUI(GameObject& owner) :
		UIRect(owner)
	{
		ImGui::SetCurrentContext(world.GetUiContext().GetContext());
		onClickListener.SetCallback(
			[this](int idx)
			{
				User& user = MapleClient::GetInstance()->GetUser();
				auto& inventory = user.GetInventory();
				if (selectedSlotIdx == idx)
					selectedSlotIdx = -1;
				else if (selectedSlotIdx == -1)
				{
					if (inventory.GetSlots()[idx].itemId != -1 && inventory.GetSlots()[idx].quantity != 0)
						selectedSlotIdx = idx;
				}
				else
				{
					InventorySlotSwapPacket packet{};
					packet.user = user.GetUserUUID();
					packet.slotA = selectedSlotIdx;
					packet.slotB = idx;
					MapleClient::GetInstance()->SendTcp(packet);

					selectedSlotIdx = -1;
				}
			}
		);
	}
	SH_USER_API void InventoryUI::Awake()
	{
		SetPriority(-3);
		for (int i = 0; i < slots.size(); ++i)
		{
			slots[i]->SetIndex(i);
			slots[i]->onClick.Register(onClickListener);
		}
	}
	SH_USER_API void InventoryUI::Update()
	{
		Dragging();
		RenderInventory();
		RenderDropWindow();
	}
	SH_USER_API void InventoryUI::OnClick()
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
	void InventoryUI::RenderInventory()
	{
		User& user = MapleClient::GetInstance()->GetUser();
		const auto& inventory = user.GetInventory();
		const auto& inventorySlots = inventory.GetSlots();
		const ItemDB& itemDB = *ItemDB::GetInstance();

		for (int i = 0; i < slots.size(); ++i)
		{
			slots[i]->GetRenderer()->gameObject.SetActive(false);
			slots[i]->GetNumberUI()->gameObject.SetActive(false);
		}

		for (int i = 0; i < slots.size(); ++i)
		{
			const int itemId = inventorySlots[i].itemId;
			if (itemId == -1)
				continue;

			const ItemInfo* const info = itemDB.GetItemInfo(itemId);
			if (info == nullptr)
				continue;

			const InventorySlotUI* slotUi = slots[i];
			auto renderer = slotUi->GetRenderer();
			if (renderer == nullptr)
				continue;

			slots[i]->GetRenderer()->gameObject.SetActive(true);
			slots[i]->GetNumberUI()->gameObject.SetActive(true);
			if (slots[i]->GetNumberUI()->GetNumber() != inventorySlots[i].quantity)
				slots[i]->GetNumberUI()->SetNumber(inventorySlots[i].quantity);

			render::Texture* itemTexture = static_cast<render::Texture*>(core::SObject::GetSObjectUsingResolver(core::UUID{ info->texUUID }));
			if (itemTexture == nullptr)
				SH_INFO_FORMAT("id: {} is null", itemId);
			else
			{
				const render::Texture* slotTex = renderer->GetMaterialPropertyBlock()->GetTextureProperty("tex");
				if (slotTex != itemTexture)
				{
					renderer->GetMaterialPropertyBlock()->SetProperty("tex", itemTexture);
					renderer->UpdatePropertyBlockData();
				}
			}
		}
	}
	void InventoryUI::Dragging()
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
	void InventoryUI::RenderDropWindow()
	{
		if (!bShowDropWindow)
			return;
		const float windowWidth = world.renderer.GetWidth();
		const float windowHeight = world.renderer.GetHeight();
		const float width = 500;
		const float height = 300;

		ImGui::SetNextWindowPos({ windowWidth / 2 - width / 2, windowHeight / 2 - height / 2 }, ImGuiCond_::ImGuiCond_Appearing);
		ImGui::SetNextWindowSize({ width, height }, ImGuiCond_::ImGuiCond_Appearing);
		ImGui::Begin("DropWindow", nullptr, ImGuiWindowFlags_::ImGuiWindowFlags_NoResize | ImGuiWindowFlags_::ImGuiWindowFlags_NoMove);
		ImGui::End();
	}
}//namespace