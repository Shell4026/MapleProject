#pragma once
#include "../Export.h"
#include "Network/Packet.h"

#include "Core/Reflection/TypeTraits.hpp"

#include <string>
#include <vector>
#include <array>
namespace sh::game
{
	class ItemDropPacket : public network::Packet
	{
		SPACKET(ItemDropPacket, ID)
	public:
		auto GetId() const -> uint32_t override
		{
			return ID;
		}
		auto Serialize() const -> core::Json
		{
			auto mainJson = network::Packet::Serialize();
			mainJson["itemUUID"] = itemUUID;
			mainJson["itemId"] = itemId;
			mainJson["x"] = x;
			mainJson["y"] = y;
			mainJson["cnt"] = cnt;
			mainJson["ownerUUID"] = ownerUUID;
			return mainJson;
		}
		void Deserialize(const core::Json& json)
		{
			network::Packet::Deserialize(json);
			if (json.contains("itemId"))
				itemId = json["itemId"];
			if (json.contains("x"))
				x = json["x"];
			if (json.contains("y"))
				y = json["y"];
			if (json.contains("cnt"))
				cnt = json["cnt"];
			if (json.contains("itemUUID"))
				itemUUID = json["itemUUID"];
			if (json.contains("ownerUUID"))
				ownerUUID = json["itemUUID"];
		}
	public:
		constexpr static uint32_t ID = (core::reflection::TypeTraits::GetTypeHash<ItemDropPacket>() >> 32);

		int itemId = 0;
		float x = 0.f;
		float y = 0.f;
		int cnt = 1;
		std::array<uint32_t, 4> itemUUID;
		std::array<uint32_t, 4> ownerUUID{ 0, 0, 0, 0 };
	};
}//namespace