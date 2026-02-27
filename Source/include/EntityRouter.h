#pragma once
#include "Export.h"
#include "Player/Player.h"

#include "Core/UUID.h"
#include "Core/SContainer.hpp"
#include "Core/EventSubscriber.h"

#include "Network/PacketEvent.hpp"

#include <unordered_map>
namespace sh::game
{
	class EntityRouter
	{
	public:
		SH_USER_API EntityRouter();

		SH_USER_API void SetPacketBus(core::EventBus& packetBus) { packetBus.Subscribe(packetSubscriber); }

		SH_USER_API void RegisterPlayer(Player& player);
		SH_USER_API void UnRegisterPlayer(Player* player);
		SH_USER_API void UnRegisterPlayer(const core::UUID& userUUID);
		SH_USER_API auto GetPlayer(const core::UUID& uuid) const -> Player*;
	private:
		core::SHashMap<core::UUID, Player*> players;
		core::EventSubscriber<network::PacketEvent> packetSubscriber;
	};
}//namespace