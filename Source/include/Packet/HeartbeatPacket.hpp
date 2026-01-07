#pragma once
#include "../Export.h"
#include "Network/Packet.h"

#include "Core/Reflection/TypeTraits.hpp"

#include <string>
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
			return mainJson;
		}
		void Deserialize(const core::Json& json)
		{
			network::Packet::Deserialize(json);
		}
	};
}//namespace