#pragma once
#include "../Export.h"
#include "Network/Packet.h"

#include "Core/Reflection/TypeTraits.hpp"

#include <string>
namespace sh::game
{
	class PlayerJoinWorldPacket : public network::Packet
	{
		SPACKET(PlayerJoinWorldPacket, ID)
	public:
		constexpr static uint32_t ID = (core::reflection::TypeTraits::GetTypeHash<PlayerJoinWorldPacket>() >> 32);
	public:
		SH_USER_API auto GetId() const->uint32_t override;
		SH_USER_API auto Serialize() const->core::Json override;
		SH_USER_API void Deserialize(const core::Json& json) override;
	public:
		std::string worldUUID;
	};
}//namespace