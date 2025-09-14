#pragma once
#include "../Export.h"
#include "Network/Packet.h"

#include "Core/Reflection/TypeTraits.hpp"

#include <string>
namespace sh::game
{
	class SkillStatePacket : public network::Packet
	{
		SPACKET(SkillStatePacket, ID)
	public:
		constexpr static uint32_t ID = (core::reflection::TypeTraits::GetTypeHash<SkillStatePacket>() >> 32);
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
			json["using"] = bUsing;
			return json;
		}
		void Deserialize(const core::Json& json) override
		{
			network::Packet::Deserialize(json);
			if (json.contains("pid"))
				userUUID = json["pid"];
			if (json.contains("sid"))
				skillId = json["sid"];
			if (json.contains("using"))
				bUsing = json["using"];
		}
	public:
		std::string userUUID;
		uint32_t skillId;
		bool bUsing;
	};
}//namespace