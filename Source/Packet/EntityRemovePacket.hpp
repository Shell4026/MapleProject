#pragma once
#include "Network/Packet.h"

#include "Core/Reflection/TypeTraits.hpp"
namespace sh::game
{
	class EntityRemovePacket : public network::Packet
	{
		SPACKET(EntityRemovePacket, ID)
	public:
		constexpr static uint32_t ID = (core::reflection::TypeTraits::GetTypeHash<EntityRemovePacket>() >> 32);
	public:
		auto GetId() const -> uint32_t override
		{
			return ID;
		}
		auto Serialize() const -> core::Json override
		{
			core::Json json = network::Packet::Serialize();
			json["uuid"] = uuid;
			return json;
		}
		void Deserialize(const core::Json& json) override
		{
			network::Packet::Deserialize(json);
			if (json.contains("uuid"))
				uuid = json["uuid"];
		}
	public:
		std::string uuid;
	};
}//namespace