#pragma once
#include "../Export.h"
#include "Network/Packet.h"

#include "Core/Reflection/TypeTraits.hpp"

#include <array>
namespace sh::game
{
	class PlayerTokenPacket : public network::Packet
	{
		SPACKET(PlayerTokenPacket, ID)
	public:
		constexpr static uint32_t ID = (core::reflection::TypeTraits::GetTypeHash<PlayerTokenPacket>() >> 32);
	public:
		SH_USER_API auto GetId() const -> uint32_t override
		{
			return ID;
		}
		SH_USER_API auto Serialize() const -> core::Json
		{
			auto mainJson = network::Packet::Serialize();
			mainJson["token"] = token;

			return mainJson;
		}
		SH_USER_API void Deserialize(const core::Json& json)
		{
			network::Packet::Deserialize(json);
			if (json.contains("token"))
				token = json["token"];
		}
	public:
		std::array<uint32_t, 4> token;
	};
}//namespace