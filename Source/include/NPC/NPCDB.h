#pragma once
#include "Export.h"
#include "NPCInfo.hpp"
#include "Game/Component/Component.h"
#include "Game/TextObject.h"

#include <unordered_map>
#include <string>
namespace sh::game
{
	class NPCDB : public Component
	{
		COMPONENT(NPCDB, "user")
	public:
		SH_USER_API NPCDB(GameObject& owner);
		SH_USER_API void Awake() override;

		SH_USER_API static auto GetNPCInfo(uint32_t id) -> const NPCInfo*;
	private:
		void Parse();
	private:
		PROPERTY(npcDB)
		TextObject* npcDB = nullptr;
		
		static std::unordered_map<uint32_t, NPCInfo> npcInfos;
	};
}//namespace