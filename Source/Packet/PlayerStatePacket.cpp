#include "PlayerStatePacket.h"

namespace sh::game
{
	SH_USER_API auto PlayerStatePacket::GetId() const -> uint32_t
	{
		return ID;
	}
	SH_USER_API auto PlayerStatePacket::Serialize() const -> core::Json
	{
		auto json = network::Packet::Serialize();
		json["px"] = px;
		json["py"] = py;
		json["vx"] = vx;
		json["vy"] = vy;
		json["lastSeq"] = lastProcessedInputSeq;
		json["player"] = playerUUID;
		json["tick"] = serverTick;
		json["ts"] = timestamp;
		json["ground"] = bGround;
		json["floor"] = floor;
		return json;
	}
	SH_USER_API void PlayerStatePacket::Deserialize(const core::Json& json)
	{
		network::Packet::Deserialize(json);
		if (json.contains("px"))
			px = json["px"];
		if (json.contains("py"))
			py = json["py"];
		if (json.contains("vx"))
			vx = json["vx"];
		if (json.contains("vy"))
			vy = json["vy"];
		if (json.contains("lastSeq"))
			lastProcessedInputSeq = json["lastSeq"];
		if (json.contains("player"))
			playerUUID = json["player"];
		if (json.contains("tick"))
			serverTick = json["tick"];
		if (json.contains("ts"))
			timestamp = json["ts"];
		if (json.contains("ground"))
			bGround = json["ground"];
		if (json.contains("floor"))
			floor = json["floor"];
	}
}//namespace