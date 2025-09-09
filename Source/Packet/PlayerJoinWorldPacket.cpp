#include "PlayerJoinWorldPacket.h"

namespace sh::game
{
	SH_USER_API auto PlayerJoinWorldPacket::GetId() const -> uint32_t
	{
		return ID;
	}
	SH_USER_API auto PlayerJoinWorldPacket::Serialize() const -> core::Json
	{
		auto mainJson = network::Packet::Serialize();
		mainJson["worldUUID"] = worldUUID;
		return mainJson;
	}
	SH_USER_API void PlayerJoinWorldPacket::Deserialize(const core::Json& json)
	{
		network::Packet::Deserialize(json);
		if (json.contains("worldUUID"))
			worldUUID = json["worldUUID"];
	}
}//namespace