#pragma once
#include "Export.h"
#include "Database.h"

#include "Core/Singleton.hpp"

#include <filesystem>
#include <unordered_map>
#include <random>
#include <cstdint>
namespace sh::game
{
	class ItemDropManager : public core::Singleton<ItemDropManager>
	{
		friend core::Singleton<ItemDropManager>;
	public:
		SH_USER_API void LoadData(const std::filesystem::path& path);
		SH_USER_API auto DropItem(int mobId) -> std::vector<int>;
	protected:
		SH_USER_API ItemDropManager();
	private:
		struct Item
		{
			int id;
			float dropRate;
		};
		std::unordered_map<int, std::vector<Item>> dropTable;
		std::mt19937 rng;

		uint64_t nextItemInstance = 0;
	};
}//namespace