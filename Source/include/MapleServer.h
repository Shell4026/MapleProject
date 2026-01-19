#pragma once
#include "Export.h"
#include "User.h"
#include "EndPoint.hpp"
#include "UserManager.h"

#include "Core/EventBus.h"
#include "Core/EventSubscriber.h"
#include "Core/EngineThread.h"

#include "Game/Component/Component.h"
#include "Game/Component/UdpServer.h"
#include "Game/WorldEvents.hpp"
#include "Game/World.h"

#include "Network/TcpListener.h"
#include "Network/MessageQueue.h"

#include <vector>
#include <unordered_map>
#include <list>
#include <optional>
#include <future>
#include <memory>
namespace sh::game
{
	class PlayerJoinPacket;
	class PlayerLeavePacket;

	class MapleServer : public UdpServer
	{
		COMPONENT(MapleServer, "user")
	public:
		SH_USER_API MapleServer(GameObject& owner);
#if SH_SERVER
		SH_USER_API void BroadCast(const network::Packet& packet);
		SH_USER_API void BroadCast(const network::Packet& packet, const Endpoint& ignore);
		SH_USER_API void BroadCast(const network::Packet& packet, const std::vector<Endpoint>& ignores);

		SH_USER_API void OnDestroy() override;
		SH_USER_API void Awake() override;
		SH_USER_API void Start() override;
		SH_USER_API void Update() override;

		SH_USER_API auto GetUserManager() -> UserManager& { return userManager; }
		SH_USER_API static auto GetInstance() -> MapleServer*;
	private:
		void ProcessUdpPacket();
		void ProcessTcpPacket();
		void ProcessTcpListen();
#endif
	public:
		core::EventBus bus;
	private:
		static MapleServer* instance;

		PROPERTY(loadedWorlds)
		std::vector<World*> loadedWorlds;

#if SH_SERVER
		core::EventSubscriber<events::ComponentEvent> componentSubscriber;
		core::EventSubscriber<UserEvent> userEventSubscriber;
		UserManager userManager;

		network::NetworkContext tcpCtx;
		network::TcpListener tcpListener;

		std::thread tcpThread;

		std::unordered_map<Endpoint, std::unique_ptr<network::TcpSocket>> pendingTcpSockets;
		std::shared_ptr<network::MessageQueue> tcpMessageQueue;

		bool bStop = false;
#endif
	};
}//namespace
