#include "World/ItemPool.h"
#include "World/MapleWorld.h"

#include "Item/Item.h"

#include <algorithm>
namespace sh::game
{
	SH_USER_API auto ItemPool::GetItem(MapleWorld& world, Prefab& itemPrefab) -> Item&
	{
		GameObject* itemObj = nullptr;
		Item* item = nullptr;
		std::size_t itemIdx = items.size();
		if (sleepItemIdx.empty())
		{
			itemObj = itemPrefab.AddToWorld(world.world);
			item = itemObj->GetComponent<Item>();
			items.push_back(ItemState{ item, false });
		}
		else
		{
			do
			{
				const std::size_t idx = sleepItemIdx.front();
				sleepItemIdx.pop();
				itemIdx = idx;
				item = items[idx].item.Get();
			} while (!core::IsValid(item) && !sleepItemIdx.empty());

			if (!core::IsValid(item))
			{
				itemObj = itemPrefab.AddToWorld(world.world);
				item = itemObj->GetComponent<Item>();
				items.push_back(ItemState{ item, false });
				itemIdx = items.size() - 1;
			}
			else
			{
				itemObj = &item->gameObject;
				items[itemIdx].bSleep = false;
				itemObj->SetUUID(core::UUID::Generate());
				itemObj->SetActive(true);
			}
		}

		item->SetCurrentWorld(world);

		return *item;
	}
	SH_USER_API void ItemPool::DestroyItem(Item& item)
	{
		item.gameObject.SetActive(false);
		
		for (std::size_t i = 0; i < items.size(); ++i)
		{
			auto& itemState = items[i];
			if (itemState.item == &item)
			{
				itemState.bSleep = true;
				sleepItemIdx.push(i);
				break;
			}
		}
	}
	SH_USER_API void ItemPool::TryClearSleepItems()
	{
		auto it = std::remove_if(items.begin(), items.end(),
			[](const ItemState& itemState)
			{
				if (itemState.bSleep && itemState.item.IsValid())
					itemState.item->gameObject.Destroy();
				return itemState.bSleep || !itemState.item.IsValid();
			}
		);
		items.erase(it, items.end());
		sleepItemIdx = std::queue<std::size_t>{};
	}
	SH_USER_API auto ItemPool::GetActiveItems() const -> std::vector<Item*>
	{
		std::vector<Item*> result;
		result.reserve(items.size());

		for (const ItemState& itemState : items)
		{
			Item* const item = itemState.item.Get();
			if (itemState.bSleep || !core::IsValid(item))
				continue;

			result.push_back(item);
		}

		return result;
	}
}//namespace
