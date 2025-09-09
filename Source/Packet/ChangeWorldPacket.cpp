#include "ChangeWorldPacket.h"

namespace sh::game
{
	SH_USER_API auto ChangeWorldPacket::GetId() const -> uint32_t
	{
		return ID;
	}
	SH_USER_API auto ChangeWorldPacket::Serialize() const -> core::Json
	{
		core::Json mainJson = Packet::Serialize();
		mainJson["uuid"] = worldUUID;
		return mainJson;
	}
	SH_USER_API void ChangeWorldPacket::Deserialize(const core::Json& json)
	{
		network::Packet::Deserialize(json);
		if (json.contains("uuid"))
			worldUUID = json["uuid"];
	}
}//namespace