#include "MapleServer.h"
#include "MapleWorld.h"
#include "Item/ItemDropManager.h"
#include "Item/ItemDB.h"
#include "PacketEvent.hpp"
#include "Packet/PlayerJoinPacket.hpp"
#include "Packet/ChangeWorldPacket.hpp"
#include "Packet/PlayerJoinSuccessPacket.hpp"
#include "Packet/PlayerJoinPacket.hpp"
#include "Packet/PlayerLeavePacket.hpp"
#include "Packet/PlayerDespawnPacket.hpp"
#include "Packet/HeartbeatPacket.hpp"

#include "Core/Util.h"
#include "Core/ThreadPool.h"

#include "Game/GameObject.h"
#include "Game/GameManager.h"
#include "Game/Component/MeshRenderer.h"
#include "Game/Component/Camera.h"

#include "Network/StringPacket.h"

namespace sh::game
{
	MapleServer* MapleServer::instance = nullptr;

	MapleServer::MapleServer(GameObject& owner) :
		UdpServer(owner)
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
				const User* user = evt.user;
				if (evt.type == UserEvent::Type::JoinUser)
				{
					{
						// 유저에게 UUID 전송
						PlayerJoinSuccessPacket packet{};
						packet.uuid = user->GetUserUUID();

						server.Send(packet, user->GetIp(), user->GetPort());
					}
					{
						// 유저 월드 이동
						World* firstWorld = loadedWorlds[0];

						ChangeWorldPacket packet{};
						packet.worldUUID = firstWorld->GetUUID();

						server.Send(packet, user->GetIp(), user->GetPort());
					}
				}
				else if (evt.type == UserEvent::Type::LeaveUser)
				{
					SH_INFO_FORMAT("A player({}) has leave - {}:{}", user->GetNickName(), user->GetIp(), user->GetPort());
				}
			}
		);

		if (instance == nullptr)
			instance = this;

		ItemDropManager::GetInstance()->LoadData("itemDrop.json");
		ItemDB::GetInstance();

		userManager.bus.Subscribe(userEventSubscriber);
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
		for (auto world : loadedWorlds)
		{
			SH_INFO_FORMAT("loading other world...({})", world->GetUUID().ToString());
			world->SubscribeEvent(componentSubscriber);
			if (world != nullptr)
				GameManager::GetInstance()->LoadWorld(world->GetUUID(), GameManager::LoadMode::Additive, true);
		}
	}
	SH_USER_API void MapleServer::BeginUpdate()
	{
		Super::BeginUpdate();
		if (!server.IsOpen())
			return;
		auto opt = server.GetReceivedMessage();
		while (opt.has_value())
		{
			auto& message = opt.value();
			//SH_INFO_FORMAT("Received packet (id: {})", message.packet->GetId());
			const std::string& ip = message.senderIp;
			const uint16_t port = message.senderPort;
			const Endpoint endpoint{ ip, port };

			if (message.packet->GetId() == PlayerJoinPacket::ID)
			{
				if (!userManager.ProcessPlayerJoin(static_cast<PlayerJoinPacket&>(*message.packet), endpoint))
				{
					network::StringPacket packet{};
					packet.SetString("The same IP address is already connected to the server.");
					server.Send(packet, endpoint.ip, endpoint.port);
				}
			}
			else if (message.packet->GetId() == PlayerLeavePacket::ID)
				userManager.ProcessPlayerLeave(static_cast<PlayerLeavePacket&>(*message.packet), endpoint);

			PacketEvent evt{};
			evt.packet = message.packet.get();
			evt.senderIp = ip;
			evt.senderPort = port;

			bus.Publish(evt);
			opt = server.GetReceivedMessage();
		}

		userManager.Update();
	}
	SH_USER_API void MapleServer::Update()
	{
	}
	SH_USER_API auto MapleServer::GetInstance() -> MapleServer*
	{
		return instance;
	}
}//namespace
