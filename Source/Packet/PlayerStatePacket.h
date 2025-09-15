#pragma once
#include "../Export.h"
#include "Network/Packet.h"

#include "Core/Reflection/TypeTraits.hpp"

#include <string>
namespace sh::game
{
	class PlayerStatePacket : public network::Packet
	{
		SPACKET(PlayerStatePacket, ID)
	public:
		constexpr static uint32_t ID = (core::reflection::TypeTraits::GetTypeHash<PlayerStatePacket>() >> 32);
	public:
		SH_USER_API auto GetId() const->uint32_t override;
		SH_USER_API auto Serialize() const->core::Json override;
		SH_USER_API void Deserialize(const core::Json& json) override;
	public:
		float px = 0.f, py = 0.f;
		float vx = 0.f, vy = 0.f;
		float floor = -1000.f;

		uint32_t lastProcessedInputSeq = 0;
		uint32_t serverTick = 0;

		uint64_t timestamp = 0;

		std::string playerUUID;

		bool bGround = false;
		bool bProne = false;
		bool bLock = false;
		bool bRight = false;
	};
}//namespace