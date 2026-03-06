#include "World/MapleWorld.h"

// 공용
namespace sh::game
{
	SH_USER_API void MapleWorld::Awake()
	{
		if (itemPrefab == nullptr)
		{
			itemPrefab = static_cast<Prefab*>(core::SObject::GetSObjectUsingResolver(core::UUID{ Item::PREFAB_UUID }));
			if (itemPrefab == nullptr)
				SH_ERROR_FORMAT("Item prefab is not valid!: {}", Item::PREFAB_UUID);
			else
			{
				core::GarbageCollection::GetInstance()->SetRootSet(itemPrefab);
			}
		}
	}
	SH_USER_API void MapleWorld::LateUpdate()
	{
		TryClearSleepItems();
		++worldTick;
	}

	void MapleWorld::TryClearSleepItems()
	{
		if (itemPool.GetSleepItemSize() == 0)
			return;

		const uint64_t noSpawnTicks = worldTick - lastItemSpawnTick;
		if (noSpawnTicks < clearSleepItemsAfterTicks)
			return;

		lastItemSpawnTick = worldTick;
		itemPool.TryClearSleepItems();
		SH_INFO("Clear sleepItems...");
	}
}//namespace