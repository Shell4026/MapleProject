#pragma once
#include "../Export.h"
#include "Network/Packet.h"

#include "Core/Reflection/TypeTraits.hpp"

#include <string>
namespace sh::game
{
	class PlayerSpawnPacket : public network::Packet
	{
		SPACKET(PlayerSpawnPacket, ID)
	public:
		constexpr static uint32_t ID = (core::reflection::TypeTraits::GetTypeHash<PlayerSpawnPacket>() >> 32);
	public:
		SH_USER_API auto GetId() const -> uint32_t override
		{
			return ID;
		}
		SH_USER_API auto Serialize() const -> core::Json
		{
			auto mainJson = network::Packet::Serialize();
			mainJson["x"] = x;
			mainJson["y"] = y;
			mainJson["player"] = playerUUID;

			return mainJson;
		}
		SH_USER_API void Deserialize(const core::Json& json)
		{
			network::Packet::Deserialize(json);
			if (json.contains("x"))
				x = json["x"];
			if (json.contains("y"))
				y = json["y"];
			if (json.contains("player"))
				playerUUID = json["player"];
		}
	public:
		float x;
		float y;
		std::string playerUUID;
	};
}//namespace