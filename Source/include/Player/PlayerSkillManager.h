#pragma once
#include "Export.h"
#include "PacketEvent.hpp"

#include "Game/Component/Component.h"

#include "Core/EventSubscriber.h"
#include "Core/SContainer.hpp"
namespace sh::game
{
	class Player;
	class Skill;
	class SkillStatePacket;

	class PlayerSkillManager : public Component
	{
		COMPONENT(PlayerSkillManager, "user")
	public:
		SH_USER_API PlayerSkillManager(GameObject& owner);

		SH_USER_API void Awake() override;

		SH_USER_API void Register(Skill& skill);
		SH_USER_API void UnRegister(Skill& skill);

		SH_USER_API void UseSkill(uint32_t id);
		
		SH_USER_API auto GetPlayer() const -> Player* { return player; }

#if !SH_SERVER
		SH_USER_API void ProcessState(uint32_t id, const SkillStatePacket& packet);
#endif
	private:
		PROPERTY(player)
		Player* player = nullptr;

		core::SMap<uint32_t, Skill*> skills;

		core::EventSubscriber<PacketEvent> packetSubscriber;
	};
}
