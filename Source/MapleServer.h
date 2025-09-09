#pragma once
#if SH_SERVER
#include "Export.h"
#include "User.h"
#include "EndPoint.hpp"

#include "Core/EventBus.h"
#include "Core/EventSubscriber.h"

#include "Game/Component/Component.h"
#include "Game/Component/UdpServer.h"
#include "Game/WorldEvents.hpp"

#include <vector>
#include <unordered_map>
namespace sh::game
{
	class PlayerJoinPacket;
	class PlayerLeavePacket;

	class MapleServer : public UdpServer
	{
		COMPONENT(MapleServer, "user")
	public:
		SH_USER_API MapleServer(GameObject& owner);

		SH_USER_API auto GetUserUUID(const Endpoint& ep) -> core::UUID*;
		SH_USER_API auto GetUser(const Endpoint& ep) -> User*;
		SH_USER_API auto GetUser(const core::UUID& uuid) -> User*;

		SH_USER_API void BroadCast(const network::Packet& packet);
		SH_USER_API void BroadCast(const network::Packet& packet, const Endpoint& ignore);
		SH_USER_API void BroadCast(const network::Packet& packet, const std::vector<Endpoint>& ignores);

		SH_USER_API void Awake() override;
		SH_USER_API void Start() override;
		SH_USER_API void BeginUpdate() override;
		SH_USER_API void Update() override;

		SH_USER_API void Kick(const User& user);
		SH_USER_API void Kick(const core::UUID& user);

		SH_USER_API static auto GetInstance() -> MapleServer*;
	private:
		void ProcessPlayerJoin(const PlayerJoinPacket& packet, const Endpoint& endpoint);
		void ProcessPlayerLeave(const PlayerLeavePacket& packet, const Endpoint& endpoint);
	public:
		core::EventBus bus;
	private:
		std::unordered_map<core::UUID, User> users;
		std::unordered_map<Endpoint, core::UUID> uuids;
		PROPERTY(loadedWorlds)
		std::vector<World*> loadedWorlds;

		core::EventSubscriber<events::ComponentEvent> componentSubscriber;

		static MapleServer* instance;
	};
}//namespace
#endif
