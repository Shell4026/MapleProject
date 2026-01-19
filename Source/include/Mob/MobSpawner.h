#pragma once
#include "Export.h"
#include "Mob.h"
#include "MobEvents.hpp"
#include "Packet/MobSpawnPacket.hpp"
#include "Packet/PlayerJoinWorldPacket.hpp"

#include "Network/PacketEvent.hpp"

#include "Game/Component/Component.h"

#include <queue>
#include <vector>
namespace sh::game
{
	class MobSpawner : public Component
	{
		COMPONENT(MobSpawner, "user")
	public:
		SH_USER_API MobSpawner(GameObject& owner);
		
		SH_USER_API void Awake() override;
		SH_USER_API void Start() override;
		SH_USER_API void Update() override;
#if SH_SERVER
		SH_USER_API void SpawnMob();
		SH_USER_API void ProcessPlayerJoin(const PlayerJoinWorldPacket& packet);
#else
		SH_USER_API void ProcessMobSpawn(const MobSpawnPacket& packet);
#endif
	private:
		PROPERTY(mobs)
		std::vector<Mob*> mobs;
		PROPERTY(mobLimit)
		int mobLimit = 10;
		PROPERTY(spawnDelayMs)
		int spawnDelayMs = 5000;
#if SH_SERVER
		std::vector<bool> spawnedMobs;
		core::EventSubscriber<MobDeathEvent> deathSubscriber;
		std::queue<int> mobQueue;
		float ms = 0.f;
#endif
		core::EventSubscriber<network::PacketEvent> packetSubscriber;
	};
}//namespace