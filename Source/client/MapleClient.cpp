#include "MapleClient.h"
#if !SH_SERVER
#include "Packet/ChangeWorldPacket.hpp"
#include "Packet/PlayerLeavePacket.hpp"
#include "Packet/InventorySyncPacket.hpp"
#include "Packet/PlayerTokenPacket.hpp"

#include "Network/PacketEvent.hpp"

#include "Game/GameObject.h"
#include "Game/GameManager.h"

namespace sh::game
{
	User MapleClient::user{ User::CreateInfo{0, "local", "127.0.0.1", 0 } };

	MapleClient* MapleClient::instance = nullptr;

	MapleClient::MapleClient(GameObject& owner) :
		UdpClient(owner),
		tcpSocket(tcpCtx)
	{
		instance = this;
		tcpMessageQueue = std::make_shared<network::MessageQueue>();
		tcpSocket.SetReceiveQueue(tcpMessageQueue);
	}
	MapleClient::~MapleClient()
	{
		tcpSocket.Close();
		if (tcpThread.joinable())
			tcpThread.join();
	}
	SH_USER_API void MapleClient::OnDestroy()
	{
		PlayerLeavePacket packet{};
		packet.user = user.GetUserUUID();

		tcpSocket.SendBlocking(packet);
		Super::OnDestroy();
	}
	SH_USER_API void MapleClient::Awake()
	{
		GameManager::GetInstance()->SetImmortalObject(gameObject);
	}
	SH_USER_API void MapleClient::Start()
	{
		Super::Start();
		tcpThread = std::thread(
			[this]()
			{
				while (tcpSocket.IsOpen())
				{
					tcpCtx.Update();
				}
			}
		);
	}
	SH_USER_API void MapleClient::Update()
	{
		user.Tick(world.deltaTime);
		ProcessUdpPackets();
		ProcessTcpPackets();
	}
	SH_USER_API void MapleClient::ConnectServer()
	{
		tcpSocket.Connect(GetServerIP(), GetServerPort());
	}
	SH_USER_API void MapleClient::SendTcp(const network::Packet& packet)
	{
		tcpSocket.Send(packet);
	}
	SH_USER_API auto MapleClient::GetUser() -> User&
	{
		return user;
	}
	SH_USER_API auto MapleClient::GetInstance() -> MapleClient*
	{
		return instance;
	}
	void MapleClient::ProcessUdpPackets()
	{
		if (socket.IsOpen())
		{
			auto received = socket.GetReceivedMessage();
			while (received.has_value())
			{
				const network::Packet* receivedPacket = received.value().packet.get();

				network::PacketEvent evt{ receivedPacket, "", 0 };
				bus.Publish(evt);

				uint32_t id = receivedPacket->GetId();
				if (id == PlayerTokenPacket::ID)
				{
					auto packet = static_cast<const PlayerTokenPacket*>(receivedPacket);
					userCi.uuid = core::UUID{ packet->token };
					SH_INFO_FORMAT("Token: {}", userCi.uuid.ToString());

					PlayerTokenPacket tcpPacket{};
					tcpPacket.token = packet->token;
					tcpSocket.Send(tcpPacket);

					User::CreateInfo ci{};
					ci.id = 0;
					ci.ip = "127.0.0.1";
					ci.port = 0;
					ci.uuid = packet->token;
					ci.nickname = "local";
					user = User{ std::move(ci) };
				}
				received = socket.GetReceivedMessage();
			}
		}
	}
	void MapleClient::ProcessTcpPackets()
	{
		if (tcpSocket.IsOpen())
		{
			auto received = tcpMessageQueue->Pop();
			while (received.has_value())
			{
				const network::Packet* receivedPacket = received.value().packet.get();
				uint32_t id = receivedPacket->GetId();
				if (id == InventorySyncPacket::ID)
				{
					auto packet = static_cast<const InventorySyncPacket*>(receivedPacket);
					user.GetInventory().Deserialize(packet->inventoryJson);
				}
				else if (id == ChangeWorldPacket::ID)
				{
					auto packet = static_cast<const ChangeWorldPacket*>(receivedPacket);
					SH_INFO_FORMAT("Move world to {}", core::UUID{ packet->worldUUID }.ToString());
					GameManager::GetInstance()->LoadWorld(core::UUID{ packet->worldUUID }, GameManager::LoadMode::Single, true);
				}

				network::PacketEvent evt{ receivedPacket, "", 0 }; // 서버 아이피와 포트는 알릴 필요 없음
				bus.Publish(evt);

				received = tcpMessageQueue->Pop();
			}
		}
	}
}//namespace
#endif