#pragma once
#include "Export.h"
#include <string>
#include <cstdint>
namespace sh::game
{
	struct NPCInfo
	{
		uint32_t id;
		std::string name;
#if !SH_SERVER
		std::string script;
#endif
	};
}//namespace