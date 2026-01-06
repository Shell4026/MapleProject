#include "MapleServer.h"
#include "MapleWorld.h"
#include "Item/ItemDropManager.h"
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
#if SH_SERVER
	MapleServer* MapleServer::instance = nullptr;

	MapleServer::MapleServer(GameObject& owner) :
		UdpServer(owner), db("Item.db")
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

		ItemDropManager::GetInstance()->LoadData("itemDrop.json");
	}
	SH_USER_API auto MapleServer::GetUser(const Endpoint& ep) -> User*
	{
		auto uuidPtr = GetUserUUID(ep);
		if (uuidPtr == nullptr)
			return nullptr;
		return GetUser(*uuidPtr);
	}
	SH_USER_API auto MapleServer::GetUserUUID(const Endpoint& ep) -> core::UUID*
	{
		auto it = uuids.find(ep);
		if (it == uuids.end())
			return nullptr;
		return &it->second;
	}
	SH_USER_API auto MapleServer::GetUser(const core::UUID& uuid) -> User*
	{
		auto it = users.find(uuid);
		if (it == users.end())
			return nullptr;
		return &it->second;
	}
	SH_USER_API void MapleServer::BroadCast(const network::Packet& packet)
	{
		for (auto& [endpoint, user] : users)
			Send(packet, user.ip, user.port);
	}
	SH_USER_API void MapleServer::BroadCast(const network::Packet& packet, const Endpoint& ignore)
	{
		for (auto& [uuid, user] : users)
		{
			if (user.ip == ignore.ip && user.port == ignore.port)
				continue;
			Send(packet, user.ip, user.port);
		}
	}
	SH_USER_API void MapleServer::BroadCast(const network::Packet& packet, const std::vector<Endpoint>& ignore)
	{
		for (auto& [uuid, user] : users)
		{
			bool bPass = true;
			for (auto& ignoreEndpoint : ignore)
			{
				if (user.ip == ignoreEndpoint.ip && user.port == ignoreEndpoint.port)
				{
					bPass = false;
					break;
				}
			}
			if (bPass)
				Send(packet, user.ip, user.port);
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

				PacketEvent evt{};
				evt.packet = message.packet.get();
				evt.senderIp = ip;
				evt.senderPort = port;

				bus.Publish(evt);

				if (message.packet->GetId() == PlayerJoinPacket::ID)
					ProcessPlayerJoin(static_cast<PlayerJoinPacket&>(*message.packet), endpoint);
				else if (message.packet->GetId() == PlayerLeavePacket::ID)
					ProcessPlayerLeave(static_cast<PlayerLeavePacket&>(*message.packet), endpoint);

				opt = server.GetReceivedMessage();
			}
		}
	}
	SH_USER_API void MapleServer::Update()
	{
	}
	SH_USER_API void MapleServer::Kick(const User& user)
	{
		uuids.erase({ user.ip, user.port });
		users.erase(user.GetUserUUID());
	}
	SH_USER_API void MapleServer::Kick(const core::UUID& user)
	{
		auto userPtr = GetUser(user);
		if (userPtr == nullptr)
			return;
		Kick(*userPtr);
	}
	SH_USER_API auto MapleServer::GetInstance() -> MapleServer*
	{
		return instance;
	}
	void MapleServer::ProcessPlayerJoin(const PlayerJoinPacket& packet, const Endpoint& endpoint)
	{
		auto it = uuids.find(endpoint);
		if (it == uuids.end())
		{
			std::string nickname{ packet.GetNickName() };
			SH_INFO_FORMAT("A player({}) has joined - {}:{}", nickname, endpoint.ip, endpoint.port);

			User user{ endpoint.ip, endpoint.port };
			user.SetNickname(std::move(nickname));
			uuids.insert_or_assign(endpoint, user.GetUserUUID());

			auto& [resultIt, success] = users.insert({ user.GetUserUUID(), std::move(user)});
			{
				// 유저에게 UUID 전송
				PlayerJoinSuccessPacket packet{};
				packet.uuid = resultIt->second.GetUserUUID();

				server.Send(packet, endpoint.ip, endpoint.port);
			}
			{
				// 유저 월드 이동
				World* firstWorld = loadedWorlds[0];

				ChangeWorldPacket packet{};
				packet.worldUUID = firstWorld->GetUUID();

				server.Send(packet, endpoint.ip, endpoint.port);
			}
		}
		else
		{
			network::StringPacket packet{};
			packet.SetString("The same IP address is already connected to the server.");
			server.Send(packet, endpoint.ip, endpoint.port);
		}
	}
	void MapleServer::ProcessPlayerLeave(const PlayerLeavePacket& packet, const Endpoint& endpoint)
	{
		auto uuidIt = uuids.find(endpoint);
		if (uuidIt == uuids.end())
			return;

		auto it = users.find(uuidIt->second);
		const User& user = it->second;
		if (user.ip == endpoint.ip && user.port == endpoint.port)
		{
			SH_INFO_FORMAT("A player({}) has leave - {}:{}", user.GetNickName(), endpoint.ip, endpoint.port);
			users.erase(it);
			uuids.erase(uuidIt);
		}
	}
#else
	MapleServer::MapleServer(GameObject& owner) :
		UdpServer(owner)
	{
	}
#endif
}//namespace
