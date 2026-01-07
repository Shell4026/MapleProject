#pragma once
#include "Export.h"
#if !SH_SERVER
#include "Core/Singleton.hpp"

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