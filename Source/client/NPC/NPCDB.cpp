#include "NPC/NPCDB.h"

#include "Game/GameObject.h"
#include "Game/GameManager.h"

namespace sh::game
{
	std::unordered_map<uint32_t, NPCInfo> NPCDB::npcInfos;

	NPCDB::NPCDB(GameObject& owner) :
		Component(owner)
	{
	}
	SH_USER_API void NPCDB::Awake()
	{
		if (npcDB != nullptr)
			Parse();
	}
	SH_USER_API auto NPCDB::GetNPCInfo(uint32_t id) -> const NPCInfo*
	{
		auto it = npcInfos.find(id);
		if (it == npcInfos.end())
			return nullptr;
		return &it->second;
	}

	void NPCDB::Parse()
	{
		const core::Json npcJsons = core::Json::parse(npcDB->text);
		auto it = npcJsons.find("NPC");
		if (it == npcJsons.end())
		{
			SH_ERROR_FORMAT("Not found \"NPC\"");
			return;
		}

		for (const auto& npcJson : *it)
		{
			NPCInfo info{};
			info.id = npcJson.value("id", 0);
			info.name = npcJson.value("name", "Unknown");
			info.script = npcJson.value("script", "Unknown");
			npcInfos.insert({ info.id, std::move(info) });
		}
	}
}//namespace