#include "Mob/MobSpawner.h"
#include "Packet/MobSpawnPacket.hpp"
#include "MapleServer.h"

#include "Game/GameObject.h"

namespace sh::game
{
	MobSpawner::MobSpawner(GameObject& owner) :
		Component(owner)
	{
		packetSubscriber.SetCallback(
			[&](const network::PacketEvent& evt)
			{
				if (evt.packet->GetId() == MobSpawnPacket::ID)
				{
					ProcessMobSpawn(static_cast<const MobSpawnPacket&>(*evt.packet));
				}
			}
		);
	}

	SH_USER_API void MobSpawner::Awake()
	{
		Super::Awake();
		MapleClient::GetInstance()->bus.Subscribe(packetSubscriber);
	}
	SH_USER_API void MobSpawner::Start()
	{
	}
	SH_USER_API void MobSpawner::Update()
	{
	}
	SH_USER_API void MobSpawner::ProcessMobSpawn(const MobSpawnPacket& packet)
	{
		Mob* mob = mobs[packet.idx];
		if (!core::IsValid(mob))
			return;

		if (mob->GetUUID() != packet.mobUUID)
		{
			mob->SetUUID(core::UUID{ packet.mobUUID });
		}

		mob->gameObject.SetActive(true);
		mob->GetStatus().Reset(mob->GetMaxHP());
		mob->Reset();
	}
}//namespace