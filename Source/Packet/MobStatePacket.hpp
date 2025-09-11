#pragma once
#include "../Export.h"
#include "Network/Packet.h"

#include "Core/Reflection/TypeTraits.hpp"

#include <string>
namespace sh::game
{
	class MobStatePacket : public network::Packet
	{
		SPACKET(MobStatePacket, ID)
	public:
		constexpr static uint32_t ID = (core::reflection::TypeTraits::GetTypeHash<MobStatePacket>() >> 32);
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
			mainJson["x"] = x;
			mainJson["y"] = y;
			mainJson["vx"] = vx;
			mainJson["vy"] = vy;
			mainJson["state"] = state;
			mainJson["hp"] = hp;
			mainJson["seq"] = seq;
			return mainJson;
		}
		void Deserialize(const core::Json& json)
		{
			network::Packet::Deserialize(json);
			if (json.contains("spawner"))
				spawnerUUID = json["spawner"];
			if (json.contains("mob"))
				mobUUID = json["mob"];
			if (json.contains("x"))
				x = json["x"];
			if (json.contains("y"))
				y = json["y"];
			if (json.contains("vx"))
				vx = json["vx"];
			if (json.contains("vy"))
				vy = json["vy"];
			if (json.contains("state"))
				state = json["state"];
			if (json.contains("hp"))
				hp = json["hp"];
			if (json.contains("seq"))
				seq = json["seq"];
		}
	public:
		std::string spawnerUUID;
		std::string mobUUID;
		float x;
		float y;
		float vx;
		float vy;
		int state;
		uint32_t hp;
		uint32_t seq;
	};
}//namespace