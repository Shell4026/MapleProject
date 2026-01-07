#pragma once
#include "../Export.h"
#include "Network/Packet.h"

#include "Core/Reflection/TypeTraits.hpp"
#include <array>
namespace sh::game
{
	class ChangeWorldPacket : public network::Packet
	{
		SPACKET(ChangeWorldPacket, ID)
	public:
		constexpr static uint32_t ID = (core::reflection::TypeTraits::GetTypeHash<ChangeWorldPacket>() >> 32);
	public:
		auto GetId() const -> uint32_t override
		{
			return ID;
		}
		auto Serialize() const -> core::Json override
		{
			core::Json mainJson = Packet::Serialize();
			mainJson["uuid"] = worldUUID;
			return mainJson;
		}
		void Deserialize(const core::Json& json) override
		{
			network::Packet::Deserialize(json);
			if (json.contains("uuid"))
				worldUUID = json["uuid"];
		}
	public:
		std::array<uint32_t, 4> worldUUID;
	};
}//namespac
