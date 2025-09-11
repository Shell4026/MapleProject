#pragma once
#include "../Export.h"
#include "Network/Packet.h"

#include "Core/Reflection/TypeTraits.hpp"

#include <string>
namespace sh::game
{
	class MobSpawnPacket : public network::Packet
	{
		SPACKET(MobSpawnPacket, ID)
	public:
		constexpr static uint32_t ID = (core::reflection::TypeTraits::GetTypeHash<MobSpawnPacket>() >> 32);
	public:
		auto GetId() const -> uint32_t override
		{
			return ID;
		}
		auto Serialize() const -> core::Json
		{
			auto mainJson = network::Packet::Serialize();
			mainJson["spawner"] = spawnerUUID;
			mainJson["mob"] = mobUUID;
			return mainJson;
		}
		void Deserialize(const core::Json& json)
		{
			network::Packet::Deserialize(json);
			if (json.contains("spawner"))
				spawnerUUID = json["spawner"];
			if (json.contains("mob"))
				mobUUID = json["mob"];
		}
	public:
		std::string spawnerUUID;
		std::string mobUUID;
	};
}//namespace