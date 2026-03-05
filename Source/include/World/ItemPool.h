#pragma once
#include "Export.h"

#include "Core/SContainer.hpp"

#include "Game/World.h"
#include "Game/Prefab.h"

#include <vector>
namespace sh::game
{
	class MapleWorld;
	class Item;
	class ItemPool
	{
	public:
		SH_USER_API auto GetItem(MapleWorld& world, Prefab& itemPrefab) -> Item&;
		SH_USER_API void DestroyItem(Item& item);
		SH_USER_API void TryClearSleepItems();
		SH_USER_API auto GetSleepItemSize() const -> std::size_t {return sleepItemIdx.size(); }
	private:
		struct ItemState
		{
			core::SObjWeakPtr<Item, void> item = nullptr;
			bool bSleep = false;
		};
		std::vector<ItemState> items;
		std::queue<std::size_t> sleepItemIdx;
	};
}//namespace