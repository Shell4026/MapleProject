#include "PlayerInputPacket.h"

namespace sh::game
{
	SH_USER_API auto PlayerInputPacket::GetId() const -> uint32_t
	{
		return ID;
	}
	SH_USER_API auto PlayerInputPacket::Serialize() const -> core::Json
	{
		auto json = network::Packet::Serialize();
		json["inputX"] = inputX;
		json["jump"] = jump;
		json["seq"] = seq;
		json["player"] = playerUUID;
		json["ts"] = timestamp;
		return json;
	}
	SH_USER_API void PlayerInputPacket::Deserialize(const core::Json& json)
	{
		network::Packet::Deserialize(json);
		if (json.contains("inputX"))
			inputX = json["inputX"];
		if (json.contains("jump"))
			jump = json["jump"];
		if (json.contains("seq"))
			seq = json["seq"];
		if (json.contains("player"))
			playerUUID = json["player"];
		if (json.contains("ts"))
			timestamp = json["ts"];
	}
}//namespace