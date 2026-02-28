#include "UI/InventoryUI.h"
#include "World/MapleClient.h"
#include "Item/ItemDB.h"
#include "Packet/InventorySlotSwapPacket.hpp"

#include "Game/World.h"
#include "Game/Input.h"

#include <queue>
namespace sh::game
{
	InventoryUI::InventoryUI(GameObject& owner) :
		UIRect(owner)
	{
		onClickListener.SetCallback(
			[this](UIRect* rect)
			{
				const InventorySlotUI* slotUI = static_cast<InventorySlotUI*>(rect);
				const int idx = slotUI->GetIndex();

				User& user = MapleClient::GetInstance()->GetUser();
				auto& inventory = user.GetInventory();
				if (selectedSlotIdx == idx)
				{
					selectedSlotIdx = -1;
					ghostItem->gameObject.SetActive(false);
				}
				else if (selectedSlotIdx == -1)
				{
					if (inventory.GetSlots()[idx].itemId != -1 && inventory.GetSlots()[idx].quantity != 0)
					{
						selectedSlotIdx = idx;
						DisplayGhostItem(inventory.GetSlots()[idx].itemId);
					}
				}
				else
				{
					InventorySlotSwapPacket packet{};
					packet.user = user.GetUserUUID();
					packet.slotA = selectedSlotIdx;
					packet.slotB = idx;
					MapleClient::GetInstance()->SendTcp(packet);

					selectedSlotIdx = -1;
					ghostItem->gameObject.SetActive(false);
				}
			}
		);
	}
	SH_USER_API void InventoryUI::Awake()
	{
		if (ghostItem == nullptr)
			SH_ERROR("GhostItem is nullptr");
		
		SetPriority(-3);
		for (int i = 0; i < slots.size(); ++i)
		{
			slots[i]->SetIndex(i);
			slots[i]->onClick.Register(onClickListener);
		}
		if (uiRoot == nullptr)
			SH_ERROR("uiRoot is nullptr!");
		else
		{
			if (uiRoot->IsActive())
				bOpen = true;
		}
	}
	SH_USER_API void InventoryUI::BeginUpdate()
	{
		if (!bOpen)
		{
			if (Input::GetKeyPressed(Input::KeyCode::I))
			{
				uiRoot->SetActive(true);
				bOpen = true;
			}
		}
		else
		{
			if (Input::GetKeyPressed(Input::KeyCode::Esc) || Input::GetKeyPressed(Input::KeyCode::I))
			{
				uiRoot->SetActive(false);
				bOpen = false;
			}
		}
	}
	SH_USER_API void InventoryUI::Update()
	{
		if (!bOpen)
			return;

		Dragging();
		RenderInventory();
		RenderDropWindow();
		MoveGhostItemToCursor();
	}
	SH_USER_API void InventoryUI::OnHover()
	{
		if (!bDragging && Input::GetMousePressed(Input::MouseType::Left))
		{
			clickedPos.x = Input::mousePosition.x;
			clickedPos.y = Input::mousePosition.y;
			lastPos = gameObject.transform->GetWorldPosition();
			bDragging = true;
		}
	}	
	void InventoryUI::RenderInventory()
	{
		User& user = MapleClient::GetInstance()->GetUser();
		const auto& inventory = user.GetInventory();
		const auto& inventorySlots = inventory.GetSlots();
		static const ItemDB& itemDB = *ItemDB::GetInstance();

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

		if (bDragging && Input::GetMouseReleased(Input::MouseType::Left))
		{
			clickedPos.x = Input::mousePosition.x;
			clickedPos.y = Input::mousePosition.y;
			bDragging = false;
		}
	}
	void InventoryUI::RenderDropWindow()
	{
		if (!bShowDropWindow)
			return;
		const float windowWidth = world.renderer.GetWidth();
		const float windowHeight = world.renderer.GetHeight();
		const float width = 500;
		const float height = 300;
	}
	void InventoryUI::DisplayGhostItem(int itemId)
	{
		static const ItemDB& itemDB = *ItemDB::GetInstance();
		ghostItem->gameObject.SetActive(true);
		const ItemInfo* const info = itemDB.GetItemInfo(itemId);
		if (info == nullptr)
			return;
		render::Texture* itemTexture = static_cast<render::Texture*>(core::SObject::GetSObjectUsingResolver(core::UUID{ info->texUUID }));
		if (itemTexture == nullptr)
			SH_INFO_FORMAT("id: {} is null", itemId);
		ghostItem->GetMaterialPropertyBlock()->SetProperty("tex", itemTexture);
		ghostItem->UpdatePropertyBlockData();
	}
	void InventoryUI::MoveGhostItemToCursor()
	{
		auto mousePos = Input::mousePosition / 100.f;
		mousePos.y = world.renderer.GetHeight() * 0.01f - mousePos.y; // 월드와 화면 y는 좌우반전
		ghostItem->gameObject.transform->SetWorldPosition(mousePos.x, mousePos.y, -1.5f);
	}
}//namespace