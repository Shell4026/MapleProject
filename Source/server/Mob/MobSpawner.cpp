#include "Mob/MobSpawner.h"
#include "World/MapleServer.h"
#include "Packet/MobSpawnPacket.hpp"

#include "Game/GameObject.h"
namespace sh::game
{
	MobSpawner::MobSpawner(GameObject& owner) :
		Component(owner)
	{
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
			[&](const network::PacketEvent& evt)
			{
				if (evt.packet->GetId() == PlayerJoinWorldPacket::ID)
				{
					ProcessPlayerJoin(static_cast<const PlayerJoinWorldPacket&>(*evt.packet));
				}
			}
		);
	}

	SH_USER_API void MobSpawner::Awake()
	{
		Super::Awake();
		int idx = 0;
		for (Mob* mob : mobs)
		{
			mobQueue.push(idx++);
			mob->evtBus.Subscribe(deathSubscriber);
		}
		spawnedMobs.resize(mobs.size(), false);

		MapleServer::GetInstance()->bus.Subscribe(packetSubscriber);
	}
	SH_USER_API void MobSpawner::Start()
	{
		SpawnMob();
	}
	SH_USER_API void MobSpawner::Update()
	{
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
	}
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
			spawnPacket.mobUUID = mob->GetUUID();
			MapleServer::GetInstance()->BroadCast(spawnPacket);

			mob->BroadcastStatePacket();
		}
	}
	SH_USER_API void MobSpawner::ProcessPlayerJoin(const PlayerJoinWorldPacket& packet)
	{
		if (world.GetUUID() != packet.worldUUID)
			return;

		const User* const userPtr = MapleServer::GetInstance()->GetUserManager().GetUser(packet.user);

		for (int i = 0; i < spawnedMobs.size(); ++i)
		{
			if (!spawnedMobs[i])
				continue;

			MobSpawnPacket spawnPacket{};
			spawnPacket.idx = i;
			spawnPacket.mobUUID = mobs[i]->GetUUID();

			MapleServer::GetInstance()->Send(spawnPacket, userPtr->GetIp(), userPtr->GetPort());
		}
	}
}//namespace