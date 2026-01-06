#pragma once
#include "../Export.h"
#include "Network/Packet.h"

#include "Core/Reflection/TypeTraits.hpp"

#include <string>
namespace sh::game
{
	class MobHitPacket : public network::Packet
	{
		SPACKET(MobHitPacket, ID)
	public:
		constexpr static uint32_t ID = (core::reflection::TypeTraits::GetTypeHash<MobHitPacket>() >> 32);
	public:
		auto GetId() const -> uint32_t override
		{
			return ID;
		}
		auto Serialize() const -> core::Json
		{
			auto mainJson = network::Packet::Serialize();
			mainJson["mob"] = mobUUID;
			mainJson["user"] = userUUID;
			mainJson["skill"] = skillId;
			return mainJson;
		}
		void Deserialize(const core::Json& json)
		{
			network::Packet::Deserialize(json);
			if (json.contains("mob"))
				mobUUID = json["mob"];
			if (json.contains("user"))
				userUUID = json["user"];
			if (json.contains("skill"))
				skillId = json["skill"];
		}
	public:
		std::string mobUUID;
		std::string userUUID;
		uint32_t skillId;
	};
}//namespace