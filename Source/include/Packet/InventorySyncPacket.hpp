#pragma once
#include "Network/Packet.h"

#include "Core/Reflection/TypeTraits.hpp"

#include <array>
namespace sh::game
{
	class InventorySyncPacket : public network::Packet
	{
		SPACKET(InventorySyncPacket, ID)
	public:
		constexpr static uint32_t ID = (core::reflection::TypeTraits::GetTypeHash<InventorySyncPacket>() >> 32);
	public:
		auto GetId() const -> uint32_t override
		{
			return ID;
		}
		auto Serialize() const -> core::Json override
		{
			core::Json json = network::Packet::Serialize();
			json["inventory"] = inventoryJson;
			return json;
		}
		void Deserialize(const core::Json& json) override
		{
			network::Packet::Deserialize(json);
			inventoryJson = json.value("inventory", core::Json());
		}
	public:
		core::Json inventoryJson;
	};
}//namespace