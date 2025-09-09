#include "PlayerJoinSuccessPacket.h"

namespace sh::game
{
	SH_USER_API auto PlayerJoinSuccessPacket::GetId() const -> uint32_t
	{
		return ID;
	}
	SH_USER_API auto PlayerJoinSuccessPacket::Serialize() const -> core::Json
	{
		core::Json mainJson = Packet::Serialize();
		mainJson["uuid"] = uuid;
		return mainJson;
	}
	SH_USER_API void PlayerJoinSuccessPacket::Deserialize(const core::Json& json)
	{
		network::Packet::Deserialize(json);
		if (json.contains("uuid"))
			uuid = json["uuid"];
	}
}//namespace