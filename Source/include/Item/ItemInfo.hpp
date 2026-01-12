#pragma once
#include "Export.h"

#include "Core/UUID.h"

#include <string>
namespace sh::game
{
	struct ItemInfo
	{
		int itemId;
		int maxStack;
#if !SH_SERVER
		core::UUID texUUID = core::UUID::GenerateEmptyUUID();
		std::string name;
		std::string desc;
#endif
	};
}//namespace