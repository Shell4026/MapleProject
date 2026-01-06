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
			json["pid"] = userUUID;
			json["sid"] = skillId;
			return json;
		}
		void Deserialize(const core::Json& json) override
		{
			network::Packet::Deserialize(json);
			if (json.contains("pid"))
				userUUID = json["pid"];
			if (json.contains("sid"))
				skillId = json["sid"];
		}
	public:
		std::array<uint32_t, 4> userUUID;
		uint32_t skillId;
	};
}//namespace