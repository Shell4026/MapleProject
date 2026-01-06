#include "MapleClient.h"
#if !SH_SERVER
#include "PacketEvent.hpp"
#include "Packet/ChangeWorldPacket.hpp"
#include "Packet/PlayerJoinSuccessPacket.hpp"
#include "Packet/PlayerLeavePacket.hpp"

#include "Game/GameObject.h"
#include "Game/GameManager.h"

namespace sh::game
{
	User MapleClient::user{ "127.0.0.1", 0 };
	MapleClient* MapleClient::instance = nullptr;

	MapleClient::MapleClient(GameObject& owner) :
		UdpClient(owner)
	{
		instance = this;
	}
	SH_USER_API void MapleClient::OnDestroy()
	{
		PlayerLeavePacket packet{};
		packet.playerUUID = user.GetUserUUID();

		client.Send(packet);
		Super::OnDestroy();
	}
	SH_USER_API void MapleClient::Awake()
	{
		Super::Awake();
		GameManager::GetInstance()->SetImmortalObject(gameObject);
	}
	SH_USER_API void MapleClient::Start()
	{
		Super::Start();
	}
	SH_USER_API void MapleClient::BeginUpdate()
	{
		Super::BeginUpdate();

		auto receivedPacket = client.GetReceivedPacket();
		while (receivedPacket != nullptr)
		{
			//SH_INFO_FORMAT("Received packet (id: {})", receivedPacket->GetId());
			PacketEvent evt{};
			evt.packet = receivedPacket.get();

			bus.Publish(evt);

			uint32_t id = receivedPacket->GetId();
			if (id == ChangeWorldPacket::ID)
			{
				auto packet = static_cast<ChangeWorldPacket*>(receivedPacket.get());
				SH_INFO_FORMAT("Move world to {}", core::UUID{ packet->worldUUID }.ToString());
				GameManager::GetInstance()->LoadWorld(core::UUID{ packet->worldUUID }, GameManager::LoadMode::Single, true);
			}
			else if (id == PlayerJoinSuccessPacket::ID)
			{
				auto pakcet = static_cast<PlayerJoinSuccessPacket*>(receivedPacket.get());
				user.SetUserUUID(core::UUID{ pakcet->uuid });
			}

			receivedPacket = client.GetReceivedPacket();
		}
	}
	SH_USER_API void MapleClient::Update()
	{
	}
	SH_USER_API auto MapleClient::GetUser() -> User&
	{
		return user;
	}
	SH_USER_API auto MapleClient::GetInstance() -> MapleClient*
	{
		return instance;
	}
}//namespace
#endif