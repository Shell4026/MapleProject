#include "MapleServer.h"
#include "MapleWorld.h"
#include "Item/ItemDropManager.h"
#include "Item/ItemDB.h"
#include "Packet/PlayerJoinPacket.hpp"
#include "Packet/ChangeWorldPacket.hpp"
#include "Packet/PlayerJoinPacket.hpp"
#include "Packet/PlayerLeavePacket.hpp"
#include "Packet/PlayerDespawnPacket.hpp"
#include "Packet/HeartbeatPacket.hpp"
#include "Packet/InventorySyncPacket.hpp"
#include "Packet/PlayerTokenPacket.hpp"
#include "Packet/InventorySlotSwapPacket.hpp"

#include "Core/Util.h"
#include "Core/ThreadPool.h"

#include "Game/GameObject.h"
#include "Game/GameManager.h"
#include "Game/Component/Render/MeshRenderer.h"
#include "Game/Component/Render/Camera.h"

#include "Network/StringPacket.h"
#include "Network/PacketEvent.hpp"

namespace sh::game
{
	MapleServer* MapleServer::instance = nullptr;

	MapleServer::MapleServer(GameObject& owner) :
		UdpServer(owner),
		tcpListener(tcpCtx)
	{
		componentSubscriber.SetCallback
		(
			[&](const events::ComponentEvent& evt)
			{
				if (evt.type == events::ComponentEvent::Type::Added)
				{
					//const auto& type = evt.component.GetType();
					//if (type.IsChildOf(game::MeshRenderer::GetStaticType()) ||
					//	type.IsChildOf(game::Camera::GetStaticType()))
					//	evt.component.Destroy();
				}
			}
		);
		userEventSubscriber.SetCallback(
			[this](const UserEvent& evt)
			{
				User* user = evt.user;
				if (evt.type == UserEvent::Type::JoinUser)
				{
					{
						InventorySyncPacket packet{};
						packet.inventoryJson = user->GetInventory().Serialize();

						user->GetTcpSocket()->Send(packet);
					}
					{
						// 유저 월드 이동
						World* firstWorld = loadedWorlds[0];
						user->SetCurrentWorldUUID(firstWorld->GetUUID());

						ChangeWorldPacket packet{};
						packet.worldUUID = firstWorld->GetUUID();

						user->GetTcpSocket()->Send(packet);
					}
				}
				else if (evt.type == UserEvent::Type::LeaveUser)
				{
					SH_INFO_FORMAT("A player({}) has leave - {}:{}", user->GetNickName(), user->GetIp(), user->GetPort());
					MapleWorld* mapleWorldPtr = MapleWorld::GetMapleWorld(evt.user->GetCurrentWorldUUID());
					if (mapleWorldPtr != nullptr)
						mapleWorldPtr->DespawnPlayer(user->GetUserUUID());
				}
			}
		);

		if (instance == nullptr)
			instance = this;

		ItemDropManager::GetInstance()->LoadData("itemDrop.json");
		ItemDB::GetInstance();

		userManager.bus.Subscribe(userEventSubscriber);

		tcpMessageQueue = std::make_shared<network::MessageQueue>();
	}
	SH_USER_API void MapleServer::BroadCast(const network::Packet& packet)
	{
		for (User* user : userManager.GetUsers())
			Send(packet, user->GetIp(), user->GetPort());
	}
	SH_USER_API void MapleServer::BroadCast(const network::Packet& packet, const Endpoint& ignore)
	{
		for (User* user : userManager.GetUsers())
		{
			if (user->GetIp() == ignore.ip && user->GetPort() == ignore.port)
				continue;
			Send(packet, user->GetIp(), user->GetPort());
		}
	}
	SH_USER_API void MapleServer::BroadCast(const network::Packet& packet, const std::vector<Endpoint>& ignore)
	{
		for (User* user : userManager.GetUsers())
		{
			bool bPass = true;
			for (auto& ignoreEndpoint : ignore)
			{
				if (user->GetIp() == ignoreEndpoint.ip && user->GetPort() == ignoreEndpoint.port)
				{
					bPass = false;
					break;
				}
			}
			if (bPass)
				Send(packet, user->GetIp(), user->GetPort());
		}
	}
	SH_USER_API void MapleServer::OnDestroy()
	{
		Super::OnDestroy();

		for (auto& [ep, tcpSocket] : pendingTcpSockets)
			tcpSocket->Close();

		bStop = true;
		tcpCtx.Stop();

		if (tcpThread.joinable())
			tcpThread.join();
	}
	SH_USER_API void MapleServer::Awake()
	{
		Super::Awake();
		world.renderer.GetWindow().UseSystemTimer(false);
		world.renderer.GetWindow().SetFps(60);
		SH_INFO("Set fps to 60");
	}
	SH_USER_API void MapleServer::Start()
	{
		Super::Start();

		tcpThread = std::thread(
			[this]()
			{
				while(!bStop)
					tcpCtx.Update();
			}
		);

		tcpListener.Listen(GetPort());

		for (auto world : loadedWorlds)
		{
			SH_INFO_FORMAT("loading other world...({})", world->GetUUID().ToString());
			world->SubscribeEvent(componentSubscriber);
			if (world != nullptr)
				GameManager::GetInstance()->LoadWorld(world->GetUUID(), GameManager::LoadMode::Additive, true);
		}
	}
	SH_USER_API void MapleServer::Update()
	{
		ProcessTcpListen();
		ProcessUdpPacket();
		ProcessTcpPacket();

		userManager.Tick(world.deltaTime);
	}
	SH_USER_API auto MapleServer::GetInstance() -> MapleServer*
	{
		return instance;
	}
	void MapleServer::ProcessUdpPacket()
	{
		if (socket.IsOpen())
		{
			auto opt = socket.GetReceivedMessage();
			while (opt.has_value())
			{
				auto& message = opt.value();
				//SH_INFO_FORMAT("Received packet (id: {})", message.packet->GetId());
				const std::string& ip = message.senderIp;
				const uint16_t port = message.senderPort;
				const Endpoint endpoint{ ip, port };

				if (message.packet->GetId() == PlayerJoinPacket::ID)
				{
					core::UUID token{ core::UUID::Generate() };
					if (!userManager.ProcessPlayerJoinUdp(static_cast<PlayerJoinPacket&>(*message.packet), token, endpoint))
					{
						network::StringPacket packet{};
						packet.SetString("The same IP address is already connected to the server.");
						socket.Send(packet, endpoint.ip, endpoint.port);
					}
					else
					{
						auto it = pendingTcpSockets.find(Endpoint{ endpoint.ip, endpoint.port });
						PlayerTokenPacket packet{};
						packet.token = token;
						socket.Send(packet, endpoint.ip, endpoint.port);
					}
				}
				else if (message.packet->GetId() == HeartbeatPacket::ID)
				{
					auto& packet = static_cast<const HeartbeatPacket&>(*message.packet);
					User* user = userManager.GetUser(packet.user);
					if (user != nullptr)
						user->IncreaseHeartbeat();
				}
				
				network::PacketEvent evt{ message.packet.get(), std::move(message.senderIp), port };
				bus.Publish(evt);

				opt = socket.GetReceivedMessage();
			}
		}
	}
	void MapleServer::ProcessTcpPacket()
	{
		auto messageOpt = tcpMessageQueue->Pop();
		while (messageOpt.has_value())
		{
			auto& message = messageOpt.value();
			std::string& ip = message.senderIp;
			const uint16_t port = message.senderPort;
			const Endpoint endpoint{ ip, port };

			if (message.packet->GetId() == PlayerTokenPacket::ID)
			{
				auto it = pendingTcpSockets.find(Endpoint{ ip, port });
				if (it != pendingTcpSockets.end())
				{
					auto packet = static_cast<const PlayerTokenPacket*>(message.packet.get());
					userManager.ProcessPlayerJoinTcp(std::move(it->second), packet->token, Endpoint{ ip, port });
					pendingTcpSockets.erase(it);
				}
			}
			else if (message.packet->GetId() == PlayerLeavePacket::ID)
				userManager.ProcessPlayerLeave(static_cast<PlayerLeavePacket&>(*message.packet));
			else if (message.packet->GetId() == InventorySlotSwapPacket::ID)
			{
				auto packet = static_cast<const InventorySlotSwapPacket*>(message.packet.get());
				auto userPtr = userManager.GetUser(packet->user);
				if (userPtr != nullptr)
				{
					if (packet->slotA != -1 && packet->slotB != -1)
						userPtr->GetInventory().SwapSlot(packet->slotA, packet->slotB);

					InventorySyncPacket syncPacket{};
					syncPacket.inventoryJson = userPtr->GetInventory().SerializeDirtySlots();
					userPtr->GetTcpSocket()->Send(syncPacket);
				}
			}

			network::PacketEvent evt{ message.packet.get(), std::move(ip), port };
			bus.Publish(evt);

			messageOpt = tcpMessageQueue->Pop();
		}
	}
	void MapleServer::ProcessTcpListen()
	{
		auto listenerOpt = tcpListener.GetJoinedSocket();
		while (listenerOpt.has_value())
		{
			auto tcpSocket = std::make_unique<network::TcpSocket>(std::move(listenerOpt.value()));
			tcpSocket->SetReceiveQueue(tcpMessageQueue);
			tcpSocket->ReadStart();

			SH_INFO_FORMAT("TCP connected (ip: {}, port: {})", tcpSocket->GetIp(), tcpSocket->GetPort());
			pendingTcpSockets.insert_or_assign(Endpoint{ tcpSocket->GetIp(), tcpSocket->GetPort() }, std::move(tcpSocket));

			listenerOpt = tcpListener.GetJoinedSocket();
		}
	}
}//namespace
