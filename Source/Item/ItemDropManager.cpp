#include "Item/ItemDropManager.h"

#include "Core/ISerializable.h"
#include "Core/FileSystem.h"
#include "Core/Logger.h"

namespace sh::game
{
	SH_USER_API void ItemDropManager::LoadData(const std::filesystem::path& path)
	{
		auto fileOpt = core::FileSystem::LoadText(path);
		if (!fileOpt.has_value())
		{
			SH_ERROR_FORMAT("Failed to load: {}", path.u8string());
			return;
		}
		core::Json dataJson = core::Json::parse(fileOpt.value());
		if (!dataJson.contains("mobs"))
		{
			SH_ERROR_FORMAT("Not found 'mobs' key: {}", path.u8string());
			return;
		}

		dropTable.clear();

		for (auto& mobJson : dataJson["mobs"])
		{
			if (!mobJson.contains("id") || !mobJson.contains("items"))
				continue;
			int mobId = mobJson["id"];
			auto it = dropTable.find(mobId);
			for (auto& itemJson : mobJson["items"])
			{
				Item item{};
				if (!itemJson.contains("id"))
					continue;
				item.id = itemJson["id"];
				if (itemJson.contains("dropRate"))
					item.dropRate = itemJson["dropRate"];
				if (it != dropTable.end())
					it->second.push_back(item);
				else
				{
					dropTable[mobId] = { item };
				}
			}
		}
		SH_INFO("Loading complete");
	}
	SH_USER_API auto ItemDropManager::DropItem(int mobId) -> std::vector<int>
	{
		static std::uniform_real_distribution<float> dist(0.f, 1.f);

		std::vector<int> result;

		auto it = dropTable.find(mobId);
		if (it == dropTable.end())
			return result;
		const std::vector<Item>& items = it->second;
		for (auto& item : items)
		{
			float rnd = dist(rng);
			if (rnd <= item.dropRate)
				result.push_back(item.id);
		}
		return result;
	}

	ItemDropManager::ItemDropManager() :
		rng(std::random_device{}())
	{
	}
}//namespace