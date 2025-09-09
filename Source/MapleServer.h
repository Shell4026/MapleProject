#pragma once
#include "Export.h"
#include "User.h"
#include "Packet/PlayerJoinPacket.h"

#include "Core/EventBus.h"
#include "Core/EventSubscriber.h"

#include "Game/Component/Component.h"
#include "Game/Component/UdpServer.h"
#include "Game/WorldEvents.hpp"

#include <vector>
#include <unordered_map>
namespace sh::game
{
	class MapleServer : public UdpServer
	{
		COMPONENT(MapleServer, "user")
	public:
		struct Endpoint
		{
			std::string ip;
			uint16_t port;

			auto operator==(const Endpoint& other) const noexcept -> bool
			{
				return ip == other.ip && port == other.port;
			}
		};
	public:
		SH_USER_API MapleServer(GameObject& owner);

		SH_USER_API auto GetUser(const Endpoint& endpoint) -> User*;
		SH_USER_API void BroadCast(const network::Packet& packet);
		SH_USER_API void BroadCast(const network::Packet& packet, const Endpoint& ignore);
		SH_USER_API void BroadCast(const network::Packet& packet, const std::vector<Endpoint>& ignores);

		SH_USER_API void Start() override;
		SH_USER_API void BeginUpdate() override;
		SH_USER_API void Update() override;

		SH_USER_API static auto GetInstance() -> MapleServer*;
	private:
		void ProcessPlayerJoin(const PlayerJoinPacket& packet, const Endpoint& endpoint);
	public:
		core::EventBus bus;
	private:
		struct EndpointHash
		{
			auto operator()(const Endpoint& e) const noexcept -> std::size_t
			{
				return sh::core::Util::CombineHash(std::hash<std::string>()(e.ip), std::hash<uint16_t>()(e.port));
			}
		};
		std::unordered_map<Endpoint, User, EndpointHash> users;
		PROPERTY(loadedWorlds)
		std::vector<World*> loadedWorlds;

		core::EventSubscriber<events::ComponentEvent> componentSubscriber;

		static MapleServer* instance;
	};
}//namespace