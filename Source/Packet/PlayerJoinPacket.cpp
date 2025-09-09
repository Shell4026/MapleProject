#include "PlayerJoinPacket.h"

namespace sh::game
{
	SH_USER_API auto PlayerJoinPacket::GetId() const -> uint32_t
	{
		return ID;
	}
	SH_USER_API auto PlayerJoinPacket::Serialize() const -> core::Json
	{
		core::Json mainJson = Packet::Serialize();
		mainJson["nickname"] = nickname;
		return mainJson;
	}
	SH_USER_API void PlayerJoinPacket::Deserialize(const core::Json& json)
	{
		network::Packet::Deserialize(json);
		if (json.contains("nickname"))
			nickname = json["nickname"];
	}
	SH_USER_API void PlayerJoinPacket::SetNickname(const std::string& name)
	{
		nickname = name;
	}
	SH_USER_API void PlayerJoinPacket::SetNickname(std::string&& name)
	{
		nickname = std::move(name);
	}
	SH_USER_API auto PlayerJoinPacket::GetNickName() const -> const std::string&
	{
		return nickname;
	}
	SH_USER_API auto PlayerJoinPacket::ReleaseNickname() -> std::string&
	{
		return nickname;
	}
}//namespace