#pragma once
#include "Network/Packet.h"

#include "Core/Reflection/TypeTraits.hpp"

#include <array>
namespace sh::game
{
	class KeyPacket : public network::Packet
	{
		SPACKET(KeyPacket, ID)
	public:
		constexpr static uint32_t ID = (core::reflection::TypeTraits::GetTypeHash<KeyPacket>() >> 32);
	public:
		auto GetId() const -> uint32_t override
		{
			return ID;
		}
		auto Serialize() const -> core::Json override
		{
			core::Json json = network::Packet::Serialize();
			json["userUUID"] = userUUID;
			json["keycode"] = keycode;
			json["bPressed"] = bPressed;
			return json;
		}
		void Deserialize(const core::Json& json) override
		{
			network::Packet::Deserialize(json);
			if (json.contains("userUUID"))
				userUUID = json["userUUID"];
			if (json.contains("keycode"))
				keycode = json["keycode"];
			if (json.contains("bPressed"))
				bPressed = json["bPressed"];
		}
	public:
		std::array<uint32_t, 4> userUUID;
		int keycode;
		bool bPressed;
	};
}//namespace