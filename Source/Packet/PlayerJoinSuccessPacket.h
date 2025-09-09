#pragma once
#include "../Export.h"
#include "Network/Packet.h"

#include "Core/Reflection/TypeTraits.hpp"

#include <string>
namespace sh::game
{
	class PlayerJoinSuccessPacket : public network::Packet
	{
		SPACKET(PlayerJoinSuccessPacket, ID)
	public:
		constexpr static uint32_t ID = (core::reflection::TypeTraits::GetTypeHash<PlayerJoinSuccessPacket>() >> 32);
	public:
		SH_USER_API auto GetId() const -> uint32_t override;
		SH_USER_API auto Serialize() const -> core::Json override;
		SH_USER_API void Deserialize(const core::Json& json) override;
	public:
		std::string uuid;
	};
}//namespace