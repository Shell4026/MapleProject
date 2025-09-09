#include "MapleServer.h"
#include "MapleWorld.h"
#include "PacketEvent.hpp"
#include "Packet/PlayerJoinPacket.h"
#include "Packet/ChangeWorldPacket.h"
#include "Packet/PlayerJoinSuccessPacket.h"

#include "Core/Util.h"

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

		if (instance == nullptr)
			instance = this;
	}
	SH_USER_API auto MapleServer::GetUser(const Endpoint& endpoint) -> User*
	{
		auto it = users.find(endpoint);
		if (it == users.end())
			return nullptr;
		return &it->second;
	}
	SH_USER_API void MapleServer::BroadCast(const network::Packet& packet)
	{
		for (auto& [endpoint, user] : users)
			Send(packet, endpoint.ip, endpoint.port);
	}
	SH_USER_API void MapleServer::BroadCast(const network::Packet& packet, const Endpoint& ignore)
	{
		for (auto& [endpoint, user] : users)
		{
			if (endpoint == ignore)
				continue;
			Send(packet, endpoint.ip, endpoint.port);
		}
	}
	SH_USER_API void MapleServer::BroadCast(const network::Packet& packet, const std::vector<Endpoint>& ignore)
	{
		for (auto& [endpoint, user] : users)
		{
			bool bPass = true;
			for (auto& ignoreEndpoint : ignore)
			{
				if (endpoint == ignoreEndpoint)
				{
					bPass = false;
					break;
				}
			}
			if (bPass)
				Send(packet, endpoint.ip, endpoint.port);
		}
	}
	SH_USER_API void MapleServer::Start()
	{
#if SH_SERVER
		Super::Start();
		for (auto world : loadedWorlds)
		{
			SH_INFO_FORMAT("loading other world...({})", world->GetUUID().ToString());
			world->SubscribeEvent(componentSubscriber);
			if (world != nullptr)
				GameManager::GetInstance()->LoadWorld(world->GetUUID(), GameManager::LoadMode::Additive, true);
		}
#endif
	}
	SH_USER_API void MapleServer::BeginUpdate()
	{
#if SH_SERVER
		Super::BeginUpdate();
		if (server.IsOpen())
		{
			auto opt = server.GetReceivedMessage();
			while (opt.has_value())
			{
				auto& message = opt.value();
				//SH_INFO_FORMAT("Received packet (id: {})", message.packet->GetId());
				const std::string& ip = message.senderIp;
				const uint16_t port = message.senderPort;
				const Endpoint endpoint{ ip, port };

				if (message.packet->GetId() == PlayerJoinPacket::ID)
					ProcessPlayerJoin(static_cast<PlayerJoinPacket&>(*message.packet), endpoint);

				PacketEvent evt{};
				evt.packet = message.packet.get();
				evt.senderIp = ip;
				evt.senderPort = port;

				bus.Publish(evt);

				opt = server.GetReceivedMessage();
			}
		}
#endif
	}
	SH_USER_API void MapleServer::Update()
	{
	}
	SH_USER_API auto MapleServer::GetInstance() -> MapleServer*
	{
		return instance;
	}
	void MapleServer::ProcessPlayerJoin(const PlayerJoinPacket& packet, const Endpoint& endpoint)
	{
#if SH_SERVER
		auto it = users.find(endpoint);
		if (it == users.end())
		{
			std::string nickname{ packet.GetNickName() };
			SH_INFO_FORMAT("A player({}) has joined - {}:{}", nickname, endpoint.ip, endpoint.port);

			User user{ endpoint.ip, endpoint.port };
			user.SetNickname(std::move(nickname));

			auto& [resultIt, success] = users.insert({ endpoint, std::move(user) });
			{
				// 유저에게 UUID 부여
				PlayerJoinSuccessPacket packet{};
				packet.uuid = resultIt->second.GetUUID().ToString();
				server.Send(packet, endpoint.ip, endpoint.port);
			}
			{
				// 유저 월드 이동
				World* firstWorld = loadedWorlds[0];

				ChangeWorldPacket packet{};
				packet.worldUUID = firstWorld->GetUUID().ToString();
				server.Send(packet, endpoint.ip, endpoint.port);
			}
		}
		else
		{
			network::StringPacket packet{};
			packet.SetString("The same IP address is already connected to the server.");
			server.Send(packet, endpoint.ip, endpoint.port);
		}
#endif
	}
}//namespace