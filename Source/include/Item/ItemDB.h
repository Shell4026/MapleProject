#pragma once
#include "Export.h"
#include "ItemInfo.hpp"

#include "Core/Singleton.hpp"
#include "Core/ISerializable.h"

#include <unordered_map>
namespace sh::game
{
	class ItemDB : public core::Singleton<ItemDB>
	{
		friend core::Singleton<ItemDB>;
	public:
		auto GetItemInfo(int id) const -> const ItemInfo*;
	protected:
		SH_USER_API ItemDB();
	private:
		std::unordered_map<int, ItemInfo> items;
	};
}//namespace