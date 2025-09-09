#pragma once
#include "../Export.h"
#include "Network/Packet.h"

#include "Core/Reflection/TypeTraits.hpp"

namespace sh::game
{
	class PlayerJoinPacket : public network::Packet
	{
		SPACKET(PlayerJoinPacket, ID)
	public:
		constexpr static uint32_t ID = (core::reflection::TypeTraits::GetTypeHash<PlayerJoinPacket>() >> 32);
	public:
		SH_USER_API auto GetId() const -> uint32_t override;
		SH_USER_API auto Serialize() const -> core::Json override;
		SH_USER_API void Deserialize(const core::Json& json) override;

		SH_USER_API void SetNickname(const std::string& name);
		SH_USER_API void SetNickname(std::string&& name);
		SH_USER_API auto GetNickName() const -> const std::string&;

		SH_USER_API auto ReleaseNickname() -> std::string&;
	private:
		std::string nickname = "player";
	};
}//namespace