#pragma once
#include "../Export.h"
#include "Network/Packet.h"

#include "Core/Reflection/TypeTraits.hpp"

#include <array>
namespace sh::game
{
	class PlayerStatePacket : public network::Packet
	{
		SPACKET(PlayerStatePacket, ID)
	public:
		constexpr static uint32_t ID = (core::reflection::TypeTraits::GetTypeHash<PlayerStatePacket>() >> 32);
	public:
		auto GetId() const -> uint32_t override
		{
			return ID;
		}
		auto Serialize() const -> core::Json override
		{
			auto json = network::Packet::Serialize();
			json["px"] = px;
			json["py"] = py;
			json["vx"] = vx;
			json["vy"] = vy;
			json["lastSeq"] = lastProcessedInputSeq;
			json["player"] = playerUUID;
			json["tick"] = serverTick;
			json["ts"] = timestamp;
			json["ground"] = bGround;
			json["floor"] = floor;
			json["prone"] = bProne;
			json["lock"] = bLock;
			json["right"] = bRight;
			return json;
		}
		void Deserialize(const core::Json& json) override
		{
			network::Packet::Deserialize(json);
			if (json.contains("px"))
				px = json["px"];
			if (json.contains("py"))
				py = json["py"];
			if (json.contains("vx"))
				vx = json["vx"];
			if (json.contains("vy"))
				vy = json["vy"];
			if (json.contains("lastSeq"))
				lastProcessedInputSeq = json["lastSeq"];
			if (json.contains("player"))
				playerUUID = json["player"];
			if (json.contains("tick"))
				serverTick = json["tick"];
			if (json.contains("ts"))
				timestamp = json["ts"];
			if (json.contains("ground"))
				bGround = json["ground"];
			if (json.contains("floor"))
				floor = json["floor"];
			if (json.contains("prone"))
				bProne = json["prone"];
			if (json.contains("lock"))
				bLock = json["lock"];
			if (json.contains("right"))
				bRight = json["right"];
		}
	public:
		float px = 0.f, py = 0.f;
		float vx = 0.f, vy = 0.f;
		float floor = -1000.f;

		uint32_t lastProcessedInputSeq = 0;
		uint32_t serverTick = 0;

		uint64_t timestamp = 0;

		std::array<uint32_t, 4> playerUUID;

		bool bGround = false;
		bool bProne = false;
		bool bLock = false;
		bool bRight = false;
	};
}//namespace