#pragma once
#include "../Export.h"
#include "Network/Packet.h"

#include "Core/Reflection/TypeTraits.hpp"

#include <array>
#include <cstdint>
namespace sh::game
{
	enum class SkillInputAction : uint8_t
	{
		Pressed = 0,
		Released = 1
	};

	class SkillUsingPacket : public network::Packet
	{
		SPACKET(SkillUsingPacket, ID)
	public:
		constexpr static uint32_t ID = (core::reflection::TypeTraits::GetTypeHash<SkillUsingPacket>() >> 32);
	public:
		auto GetId() const -> uint32_t override
		{
			return ID;
		}
		auto Serialize() const -> core::Json override
		{
			core::Json json = network::Packet::Serialize();
			json["seq"] = seq;
			json["tick"] = tick;
			json["uid"] = userUUID;
			json["sid"] = skillId;
			json["act"] = static_cast<uint32_t>(action);
			json["dir"] = dir;
			return json;
		}
		void Deserialize(const core::Json& json) override
		{
			network::Packet::Deserialize(json);
			if (json.contains("uid"))
				userUUID = json["uid"];
			seq = json.value("seq", 0);
			tick = json.value("tick", static_cast<uint64_t>(0));
			skillId = json.value("sid", 0);
			const uint32_t actionValue = json.value("act", static_cast<uint32_t>(SkillInputAction::Pressed));
			action = actionValue == static_cast<uint32_t>(SkillInputAction::Released) ?
				SkillInputAction::Released :
				SkillInputAction::Pressed;
			dir = json.value("dir", 0);
		}
public:
		std::array<uint32_t, 4> userUUID;
		uint32_t seq;
		uint64_t tick = 0;
		uint32_t skillId;
		SkillInputAction action = SkillInputAction::Pressed;
		int dir;
	};
}//namespace
