#pragma once
#if !SH_SERVER
#include "Export.h"
#include "User.h"

#include "Game/Component/UdpClient.h"

#include "Network/TcpSocket.h"
#include "Network/MessageQueue.h"

#include "Core/UUID.h"
#include "Core/EventBus.h"

#include <thread>
namespace sh::game
{
	class MapleClient : public UdpClient
	{
		COMPONENT(MapleClient, "user")
	public:
		SH_USER_API MapleClient(GameObject& owner);

		SH_USER_API void OnDestroy() override;
		SH_USER_API void Awake() override;
		SH_USER_API void Start() override;
		SH_USER_API void Update() override;

		SH_USER_API void ConnectServer();
		SH_USER_API void SendTcp(const network::Packet& packet);

		SH_USER_API static auto GetUser() -> User&;
		SH_USER_API static auto GetInstance() -> MapleClient*;
	private:
		void ProcessUdpPackets();
		void ProcessTcpPackets();
	public:
		core::EventBus bus;
	private:
		static User user;
		static MapleClient* instance;

		network::NetworkContext tcpCtx;
		network::TcpSocket tcpSocket;
		std::shared_ptr<network::MessageQueue> tcpMessageQueue;

		std::thread tcpThread;

		User::CreateInfo userCi;
	};
}//namespace
#endif
