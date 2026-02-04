#pragma once
#include "../Export.h"
#include "Network/Packet.h"

#include "Core/Reflection/TypeTraits.hpp"

#include <array>
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
			mainJson["nickname"] = nickname;

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
			if (json.contains("nickname"))
				nickname = json["nickname"];
		}
	public:
		float x;
		float y;
		std::array<uint32_t, 4> playerUUID;
		std::string nickname;
	};
}//namespace