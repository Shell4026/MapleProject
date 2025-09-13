#pragma once
#include "../Export.h"
#include "Network/Packet.h"

#include "Core/Reflection/TypeTraits.hpp"

#include <string>
namespace sh::game
{
	class PlayerInputPacket : public network::Packet
	{
		SPACKET(PlayerInputPacket, ID)
	public:
		constexpr static uint32_t ID = (core::reflection::TypeTraits::GetTypeHash<PlayerInputPacket>() >> 32);
	public:
		SH_USER_API auto GetId() const -> uint32_t override;
		SH_USER_API auto Serialize() const -> core::Json override;
		SH_USER_API void Deserialize(const core::Json& json) override;
	public:
		float inputX = 0.0f; // -1..1
		uint32_t seq = 0;
		uint64_t timestamp = 0; // 나중에 쓸지도
		std::string playerUUID;
		bool bJump = false;
	};
}
