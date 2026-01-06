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
		auto GetId() const -> uint32_t override
		{
			return ID;
		}
		auto Serialize() const -> core::Json override
		{
			core::Json mainJson = Packet::Serialize();
			mainJson["nickname"] = nickname;
			return mainJson;
		}
		void Deserialize(const core::Json& json) override
		{
			network::Packet::Deserialize(json);
			if (json.contains("nickname"))
				nickname = json["nickname"];
		}

		void SetNickname(const std::string& name) { nickname = name; }
		void SetNickname(std::string&& name) { nickname = std::move(name); }
		auto GetNickName() const -> const std::string& { return nickname; }

		auto ReleaseNickname() -> std::string& { return nickname; }
	private:
		std::string nickname = "player";
	};
}//namespace