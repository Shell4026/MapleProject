#pragma once
#include "Export.h"
#include "PlayerAnimation.h"
#include "PacketEvent.hpp"
#include "AI/AIStrategy.h"

#include "Core/SContainer.hpp"
#include "Core/EventSubscriber.h"

#include "Game/Component/NetworkComponent.h"
#include "Game/Component/RigidBody.h"
#include "Game/Prefab.h"
namespace sh::game
{
	class MobSpawnPacket;
	class MobStatePacket;
	class PlayerJoinWorldPacket;

	class Mob : public NetworkComponent
	{
		COMPONENT(Mob, "user")
	public:
		SH_USER_API Mob(GameObject& owner);

		SH_USER_API void Awake() override;
		SH_USER_API void Update() override;

		SH_USER_API void SetAIStrategy(AIStrategy* strategy);
		SH_USER_API auto GetAI() const -> AIStrategy*;
		SH_USER_API auto GetRigidbody() const -> RigidBody*;
		SH_USER_API auto GetAnim() const -> PlayerAnimation*;

		SH_USER_API auto GetId() const -> uint32_t;
		SH_USER_API auto GetHp() const -> uint32_t;
		SH_USER_API auto GetMaxHp() const -> int32_t;
		SH_USER_API auto GetSpeed() const -> float;
		SH_USER_API auto GetAttack() const->uint32_t;
		SH_USER_API auto GetLevel() const -> uint32_t;
		SH_USER_API auto GetExp() const -> uint32_t;
	private:
#if !SH_SERVER
		void ProcessMobSpawn(const MobSpawnPacket& packet);
		void ProcessState(const MobStatePacket& packet);
#else
		void SpawnMob();
		void ProcessPlayerJoin(const PlayerJoinWorldPacket& packet);
#endif
	protected:
		PROPERTY(id)
		uint32_t id = 0;
		PROPERTY(maxHp)
		uint32_t maxHp = 15;
		PROPERTY(speed)
		float speed = 0.6f;
		PROPERTY(attack)
		uint32_t attack = 1;
		PROPERTY(level)
		uint32_t level = 1;
		PROPERTY(exp)
		uint32_t exp = 3;
	private:
#if !SH_SERVER
		glm::vec2 serverPos;
		glm::vec2 serverVel;
		uint32_t lastStateSeq = 0;
#else
		uint64_t seq = 1;
#endif
		PROPERTY(clone, core::PropertyOption::invisible)
		Prefab* clone = nullptr;

		uint32_t hp = 15;

		PROPERTY(anim)
		PlayerAnimation* anim = nullptr;
		PROPERTY(ai)
		AIStrategy* ai = nullptr;
		PROPERTY(rigidbody)
		RigidBody* rigidbody = nullptr;

		uint32_t tick = 0;
		
		core::EventSubscriber<PacketEvent> packetSubscriber;

		core::UUID spawnerUUID;
		bool bSpawner = true;
	};
}//namespace