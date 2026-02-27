#pragma once
#include "../Export.h"
#include "Network/Packet.h"

#include "Core/Reflection/TypeTraits.hpp"

#include <array>
namespace sh::game
{
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
			json["uid"] = userUUID;
			json["sid"] = skillId;
			json["dir"] = dir;
			return json;
		}
		void Deserialize(const core::Json& json) override
		{
			network::Packet::Deserialize(json);
			if (json.contains("uid"))
				userUUID = json["uid"];
			seq = json.value("seq", 0);
			skillId = json.value("sid", 0);
			dir = json.value("dir", 0);
		}
	public:
		std::array<uint32_t, 4> userUUID;
		uint32_t seq;
		uint32_t skillId;
		int dir;
	};
}//namespace