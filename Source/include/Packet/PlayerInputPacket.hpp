#pragma once
#include "../Export.h"
#include "Network/Packet.h"

#include "Core/Reflection/TypeTraits.hpp"

#include <array>
namespace sh::game
{
	class PlayerInputPacket : public network::Packet
	{
		SPACKET(PlayerInputPacket, ID)
	public:
		constexpr static uint32_t ID = (core::reflection::TypeTraits::GetTypeHash<PlayerInputPacket>() >> 32);
	public:
		auto GetId() const -> uint32_t override
		{
			return ID;
		}
		auto Serialize() const -> core::Json override
		{
			auto json = network::Packet::Serialize();
			json["inputX"] = inputX;
			json["jump"] = bJump;
			json["seq"] = seq;
			json["user"] = user;
			json["prone"] = bProne;
			return json;
		}
		void Deserialize(const core::Json& json) override
		{
			network::Packet::Deserialize(json);
			if (json.contains("inputX"))
				inputX = json["inputX"];
			if (json.contains("jump"))
				bJump = json["jump"];
			if (json.contains("seq"))
				seq = json["seq"];
			if (json.contains("user"))
				user = json["user"];
			if (json.contains("prone"))
				bProne = json["prone"];
		}
	public:
		std::array<uint32_t, 4> user;
		float inputX = 0.0f; // -1..1
		uint32_t seq = 0;
		bool bJump = false;
		bool bProne = false;
	};
}
