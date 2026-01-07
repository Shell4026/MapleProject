#pragma once
#include "Network/Packet.h"

#include "Core/Reflection/TypeTraits.hpp"

#include <array>
namespace sh::game
{
	class ItemDespawnPacket : public network::Packet
	{
		SPACKET(ItemDespawnPacket, ID)
	public:
		constexpr static uint32_t ID = (core::reflection::TypeTraits::GetTypeHash<ItemDespawnPacket>() >> 32);
	public:
		auto GetId() const -> uint32_t override
		{
			return ID;
		}
		auto Serialize() const -> core::Json override
		{
			core::Json json = network::Packet::Serialize();
			json["uuid"] = itemObjectUUID;
			return json;
		}
		void Deserialize(const core::Json& json) override
		{
			network::Packet::Deserialize(json);
			if (json.contains("uuid"))
				itemObjectUUID = json["uuid"];
		}
	public:
		std::array<uint32_t, 4> itemObjectUUID;
	};
}//namespace