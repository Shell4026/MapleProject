#pragma once
#include "../Export.h"
#include "Network/Packet.h"

#include "Core/Reflection/TypeTraits.hpp"

#include <array>
namespace sh::game
{
	class PlayerJoinSuccessPacket : public network::Packet
	{
		SPACKET(PlayerJoinSuccessPacket, ID)
	public:
		constexpr static uint32_t ID = (core::reflection::TypeTraits::GetTypeHash<PlayerJoinSuccessPacket>() >> 32);
	public:
		auto GetId() const -> uint32_t override
		{
			return ID;
		}
		auto Serialize() const -> core::Json override
		{
			core::Json mainJson = Packet::Serialize();
			mainJson["uuid"] = uuid;
			return mainJson;
		}
		void Deserialize(const core::Json& json) override
		{
			network::Packet::Deserialize(json);
			if (json.contains("uuid"))
				uuid = json["uuid"];
		}
	public:
		std::array<uint32_t, 4> uuid;
	};
}//namespace