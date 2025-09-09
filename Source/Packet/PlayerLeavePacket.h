#pragma once
#include "../Export.h"
#include "Network/Packet.h"

#include "Core/Reflection/TypeTraits.hpp"

#include <string>
namespace sh::game
{
	class PlayerLeavePacket : public network::Packet
	{
		SPACKET(PlayerLeavePacket, ID)
	public:
		constexpr static uint32_t ID = (core::reflection::TypeTraits::GetTypeHash<PlayerLeavePacket>() >> 32);
	public:
		auto GetId() const -> uint32_t override
		{
			return ID;
		}
		auto Serialize() const -> core::Json override
		{
			core::Json json = network::Packet::Serialize();
			json["p"] = playerUUID;
			return json;
		}
		void Deserialize(const core::Json& json) override
		{
			network::Packet::Deserialize(json);
			if (json.contains("p"))
				playerUUID = json["p"];
		}
	public:
		std::string playerUUID;
	};
}//namespace