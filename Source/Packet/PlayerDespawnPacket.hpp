#pragma once
#include "../Export.h"
#include "Network/Packet.h"

#include "Core/Reflection/TypeTraits.hpp"

#include <array>
namespace sh::game
{
	class PlayerDespawnPacket : public network::Packet
	{
		SPACKET(PlayerDespawnPacket, ID)
	public:
		constexpr static uint32_t ID = (core::reflection::TypeTraits::GetTypeHash<PlayerDespawnPacket>() >> 32);
	public:
		auto GetId() const -> uint32_t override
		{
			return ID;
		}
		auto Serialize() const -> core::Json
		{
			auto mainJson = network::Packet::Serialize();
			mainJson["p"] = playerUUID;

			return mainJson;
		}
		void Deserialize(const core::Json& json)
		{
			network::Packet::Deserialize(json);
			if (json.contains("p"))
				playerUUID = json["p"];
		}
	public:
		std::array<uint32_t, 4> playerUUID;
	};
}//namespace