#pragma once
#include "Network/Packet.h"

#include "Core/Reflection/TypeTraits.hpp"

#include <array>
namespace sh::game
{
	class InventorySlotSwapPacket : public network::Packet
	{
		SPACKET(InventorySlotSwapPacket, ID)
	public:
		constexpr static uint32_t ID = (core::reflection::TypeTraits::GetTypeHash<InventorySlotSwapPacket>() >> 32);
	public:
		auto GetId() const -> uint32_t override
		{
			return ID;
		}
		auto Serialize() const -> core::Json override
		{
			core::Json json = network::Packet::Serialize();
			json["user"] = user;
			json["slotA"] = slotA;
			json["slotB"] = slotB;
			return json;
		}
		void Deserialize(const core::Json& json) override
		{
			network::Packet::Deserialize(json);
			user = json.value("user", std::array<uint32_t, 4>{0, 0, 0, 0});
			slotA = json.value("slotA", -1);
			slotB = json.value("slotB", -1);
		}
	public:
		std::array<uint32_t, 4> user;
		int slotA;
		int slotB;
	};
}//namespace