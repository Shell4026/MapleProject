#include "UI/InventoryUI.h"
#include "MapleClient.h"
#include "Item/ItemDB.h"
#include "Packet/InventorySlotSwapPacket.hpp"

#include "Game/GameObject.h"

namespace sh::game
{
	InventoryUI::InventoryUI(GameObject& owner) :
		UI(owner)
	{
		onClickListener.SetCallback(
			[this](int idx)
			{
				SH_INFO_FORMAT("click: {}", idx);
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
		for (int i = 0; i < slots.size(); ++i)
		{
			slots[i]->SetIndex(i);
			slots[i]->onClick.Register(onClickListener);
		}
	}
	SH_USER_API void InventoryUI::OnEnable()
	{
		//RenderInventory();
	}
	SH_USER_API void InventoryUI::Update()
	{
		RenderInventory();
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
}//namespace