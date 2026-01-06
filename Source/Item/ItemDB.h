#pragma once
#include "Export.h"
#include "Core/Singleton.hpp"
#if !SH_SERVER

#include "Core/ISerializable.h"

#include <unordered_map>
namespace sh::game
{
	class ItemDB : public core::Singleton<ItemDB>
	{
		friend core::Singleton<ItemDB>;
	public:
		auto GetItemInfo(int id) const -> const core::Json*;
	protected:
		SH_USER_API ItemDB();
	private:
		std::unordered_map<int, core::Json> items;
	};
}//namespace
#endif