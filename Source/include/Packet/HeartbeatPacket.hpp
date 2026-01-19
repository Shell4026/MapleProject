#pragma once
#include "../Export.h"
#include "Network/Packet.h"

#include "Core/Reflection/TypeTraits.hpp"

#include <array>
namespace sh::game
{
	class HeartbeatPacket : public network::Packet
	{
		SPACKET(HeartbeatPacket, ID)
	public:
		constexpr static uint32_t ID = (core::reflection::TypeTraits::GetTypeHash<HeartbeatPacket>() >> 32);
	public:
		auto GetId() const -> uint32_t override
		{
			return ID;
		}
		auto Serialize() const -> core::Json
		{
			auto mainJson = network::Packet::Serialize();
			mainJson["user"] = user;
			return mainJson;
		}
		void Deserialize(const core::Json& json)
		{
			network::Packet::Deserialize(json);
			if (json.contains("user"))
				user = json["user"];
		}
	public:
		std::array<uint32_t, 4> user;
	};
}//namespace