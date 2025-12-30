#include "MobSpawner.h"
#include "../Packet/MobSpawnPacket.hpp"
#include "../MapleServer.h"

#include "Game/GameObject.h"

namespace sh::game
{
	MobSpawner::MobSpawner(GameObject& owner) :
		Component(owner)
	{
#if SH_SERVER
		deathSubscriber.SetCallback(
			[&](const MobDeathEvent& evt)
			{
				auto it = std::find(mobs.begin(), mobs.end(), &evt.mob);
				if (it == mobs.end())
					return;
				int idx = std::distance(mobs.begin(), it);

				if (idx >= 0 && idx < spawnedMobs.size() && spawnedMobs[idx])
				{
					spawnedMobs[idx] = false;
					mobQueue.push(idx);
				}
			}
		);
		packetSubscriber.SetCallback(
			[&](const PacketEvent& evt)
			{
				if (evt.packet->GetId() == PlayerJoinWorldPacket::ID)
				{
					ProcessPlayerJoin(static_cast<const PlayerJoinWorldPacket&>(*evt.packet), evt.senderIp, evt.senderPort);
				}
			}
		);
#else
		packetSubscriber.SetCallback(
			[&](const PacketEvent& evt)
			{
				if (evt.packet->GetId() == MobSpawnPacket::ID)
				{
					ProcessMobSpawn(static_cast<const MobSpawnPacket&>(*evt.packet));
				}
			}
		);
#endif
	}

	SH_USER_API void MobSpawner::Awake()
	{
		Super::Awake();
#if SH_SERVER
		int idx = 0;
		for (Mob* mob : mobs)
		{
			mobQueue.push(idx++);
			mob->evtBus.Subscribe(deathSubscriber);
		}
		spawnedMobs.resize(mobs.size(), false);

		MapleServer::GetInstance()->bus.Subscribe(packetSubscriber);
#else
		MapleClient::GetInstance()->bus.Subscribe(packetSubscriber);
#endif
	}
	SH_USER_API void MobSpawner::Start()
	{
#if SH_SERVER
		SpawnMob();
#endif
	}
	SH_USER_API void MobSpawner::Update()
	{
#if SH_SERVER
		if (mobQueue.empty())
		{
			ms = 0.f;
			return;
		}

		ms += world.deltaTime * 1000.f;
		if (ms >= spawnDelayMs)
		{
			ms -= spawnDelayMs;
			SpawnMob();
		}
#endif
	}
#if SH_SERVER
	SH_USER_API void MobSpawner::SpawnMob()
	{
		int alive = std::count(spawnedMobs.begin(), spawnedMobs.end(), true);
		int desired = std::min(mobLimit, (int)mobs.size());

		while (alive < desired && !mobQueue.empty())
		{
			int spawnIdx = mobQueue.front();
			mobQueue.pop();

			if (spawnIdx < 0 || spawnIdx >= mobs.size()) 
				continue;

			Mob* mob = mobs[spawnIdx];
			if (!core::IsValid(mob)) 
				continue;

			SH_INFO_FORMAT("spawn: {}", spawnIdx);
			mob->gameObject.SetActive(true);
			mob->GetStatus().Reset(mob->GetMaxHP());
			mob->Reset();

			spawnedMobs[spawnIdx] = true;
			++alive;

			MobSpawnPacket spawnPacket{};
			spawnPacket.idx = spawnIdx;
			spawnPacket.mobUUID = mob->GetUUID().ToString();
			MapleServer::GetInstance()->BroadCast(spawnPacket);

			mob->BroadcastStatePacket();
		}
	}
	SH_USER_API void MobSpawner::ProcessPlayerJoin(const PlayerJoinWorldPacket& packet, const std::string ip, uint16_t port)
	{
		if (world.GetUUID() != packet.worldUUID)
			return;

		for (int i = 0; i < spawnedMobs.size(); ++i)
		{
			if (!spawnedMobs[i])
				continue;

			MobSpawnPacket spawnPacket{};
			spawnPacket.idx = i;
			spawnPacket.mobUUID = mobs[i]->GetUUID().ToString();

			MapleServer::GetInstance()->Send(spawnPacket, ip, port);
		}
	}
#else
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
#endif
}//namespace