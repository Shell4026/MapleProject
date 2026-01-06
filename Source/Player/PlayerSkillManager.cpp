#include "PlayerSkillManager.h"
#include "MapleServer.h"
#include "Packet/SkillUsingPacket.hpp"
#include "Packet/SkillStatePacket.hpp"
#include "Skill.h"
namespace sh::game
{
	PlayerSkillManager::PlayerSkillManager(GameObject& owner) :
		Component(owner)
	{
#if SH_SERVER
		packetSubscriber.SetCallback(
			[&](const PacketEvent& evt)
			{
				if (evt.packet->GetId() == SkillUsingPacket::ID)
				{
					auto packet = static_cast<const SkillUsingPacket*>(evt.packet);
					if (player->GetUserUUID() == packet->userUUID)
						UseSkill(packet->skillId);
				}
			}
		);
#else
		packetSubscriber.SetCallback(
			[&](const PacketEvent& evt)
			{
				if (evt.packet->GetId() == SkillStatePacket::ID)
				{
					auto packet = static_cast<const SkillStatePacket*>(evt.packet);
					ProcessState(packet->skillId, *packet);
				}
			}
		);
#endif
	}
	SH_USER_API void PlayerSkillManager::Awake()
	{
#if SH_SERVER
		MapleServer* server = MapleServer::GetInstance();
		server->bus.Subscribe(packetSubscriber);
#else
		MapleClient* client = MapleClient::GetInstance();
		client->bus.Subscribe(packetSubscriber);
#endif
	}
	SH_USER_API void PlayerSkillManager::Register(Skill& skill)
	{
		skills.insert_or_assign(skill.GetId(), &skill);
	}
	SH_USER_API void PlayerSkillManager::UnRegister(Skill& skill)
	{
		skills.erase(skill.GetId());
	}
	SH_USER_API void PlayerSkillManager::UseSkill(uint32_t id)
	{
		auto it = skills.find(id);
		if (it == skills.end())
			return;

		it->second->Use();
	}
	SH_USER_API auto PlayerSkillManager::GetPlayer() const -> Player*
	{
		return player;
	}
#if !SH_SERVER
	SH_USER_API void PlayerSkillManager::ProcessState(uint32_t id, const SkillStatePacket& packet)
	{
		auto it = skills.find(id);
		if (it == skills.end())
			return;

		it->second->ProcessState(packet);
	}
#endif
}//namespace