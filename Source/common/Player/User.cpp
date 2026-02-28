#include "Player/User.h"

#if !SH_SERVER
#include "World/MapleClient.h"
#include "Packet/HeartbeatPacket.hpp"
#endif
namespace sh::game
{
	User::User(CreateInfo&& ci) :
		id(ci.id), tcpSocket(std::move(ci.tcpSocket)), ip(std::move(ci.ip)), port(ci.port), uuid(ci.uuid), 
		nickname(std::move(ci.nickname)), inventory(std::move(ci.inventory)),
		currentWorld(core::UUID::Generate())
	{
	}
	User::User(User&& other) noexcept :
		id(other.id), tcpSocket(std::move(other.tcpSocket)), ip(other.ip), port(other.port),
		uuid(std::move(other.uuid)), currentWorld(std::move(other.currentWorld)), nickname(std::move(other.nickname)), inventory(std::move(other.inventory))
	{
	}
	SH_USER_API auto User::operator=(User&& other) noexcept -> User&
	{
		id = other.id;
		tcpSocket = std::move(other.tcpSocket);
		ip = std::move(other.ip);
		port = other.port;
		uuid = other.uuid;
		currentWorld = other.currentWorld;
		nickname = std::move(other.nickname);
		inventory = std::move(other.inventory);
		return *this;
	}
	SH_USER_API void User::SetNickname(const std::string& name)
	{
		nickname = name;
	}
	SH_USER_API void User::SetNickname(std::string&& name) noexcept
	{
		nickname = std::move(name);
	}
	SH_USER_API void User::SetCurrentWorldUUID(const core::UUID& worldUUID)
	{
		currentWorld = worldUUID;
	}
	SH_USER_API auto User::ConsumePendingSpawnPortalId() -> int
	{
		const int portalId = pendingSpawnPortalId;
		pendingSpawnPortalId = -1;
		return portalId;
	}
	SH_USER_API void User::IncreaseHeartbeat()
	{
		heartbeat = 10;
	}
	SH_USER_API void User::Tick(float dt)
	{
		if (portalTransferCooldownMs > 0.f)
		{
			portalTransferCooldownMs -= (dt * 1000.f);
			if (portalTransferCooldownMs < 0.f)
				portalTransferCooldownMs = 0.f;
		}

#if SH_SERVER
		static float t = 0.f;
		t += dt;
		if (t >= 1.f)
		{
			heartbeat--;
			t = 0.f;
		}
#else
		static float t = 0.f;
		t += dt;
		if (t >= 1.f)
		{
			HeartbeatPacket packet{};
			packet.user = uuid;
			MapleClient::GetInstance()->SendPacket(packet);
			t = 0.f;
		}
#endif
	}
}//namespace
