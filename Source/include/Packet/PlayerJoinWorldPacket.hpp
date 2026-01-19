#pragma once
#include "../Export.h"
#include "Network/Packet.h"

#include "Core/Reflection/TypeTraits.hpp"

#include <array>
namespace sh::game
{
	class PlayerJoinWorldPacket : public network::Packet
	{
		SPACKET(PlayerJoinWorldPacket, ID)
	public:
		constexpr static uint32_t ID = (core::reflection::TypeTraits::GetTypeHash<PlayerJoinWorldPacket>() >> 32);
	public:
		auto GetId() const -> uint32_t override
		{
			return ID;
		}
		auto Serialize() const -> core::Json override
		{
			auto mainJson = network::Packet::Serialize();
			mainJson["user"] = user;
			mainJson["worldUUID"] = worldUUID;
			return mainJson;
		}
		void Deserialize(const core::Json& json) override
		{
			network::Packet::Deserialize(json);
			if (json.contains("user"))
				user = json["user"];
			if (json.contains("worldUUID"))
				worldUUID = json["worldUUID"];
		}
	public:
		std::array<uint32_t, 4> user;
		std::array<uint32_t, 4> worldUUID;
	};
}//namespace