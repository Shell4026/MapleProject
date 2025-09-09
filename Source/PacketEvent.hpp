#pragma once
#include "Core/IEvent.h"
#include "Core/Reflection/TypeTraits.hpp"

#include "Network/Packet.h"

#include <memory>

namespace sh::game
{
	class PacketEvent : public core::IEvent
	{
	public:
		auto GetTypeHash() const -> std::size_t
		{
			return core::reflection::TypeTraits::GetTypeHash<PacketEvent>();
		};
	public:
		const network::Packet* packet;
		std::string senderIp;
		uint16_t senderPort;
	};
}//namespace;