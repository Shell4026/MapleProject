#pragma once
#include "Export.h"
#include "MobAnimation.h"
#include "PacketEvent.hpp"
#include "AI/AIStrategy.h"

#include "Core/SContainer.hpp"
#include "Core/EventSubscriber.h"

#include "Game/Component/NetworkComponent.h"
#include "Game/Component/RigidBody.h"
#include "Game/Prefab.h"

//#define SH_SERVER 1
namespace sh::game
{
	class MobSpawnPacket;
	class MobStatePacket;
	class PlayerJoinWorldPacket;
	class Skill;

	/// @brief 몹 클래스. 처음 맵에 배치된 개체는 스포너 개체가 된다.
	class Mob : public NetworkComponent
	{
		COMPONENT(Mob, "user")
	public:

		SH_USER_API Mob(GameObject& owner);

		SH_USER_API void Awake() override;
		SH_USER_API void Update() override;

#if !SH_SERVER
		SH_USER_API void SetAnimation(MobAnimation& anim);
		SH_USER_API auto GetAnimation() const ->MobAnimation*;
#else
		SH_USER_API void Hit(Skill& skill, Player& player);
#endif

		SH_USER_API void SetAIStrategy(AIStrategy* strategy);
		SH_USER_API auto GetAI() const -> AIStrategy*;
		SH_USER_API auto GetRigidbody() const -> RigidBody*;

		SH_USER_API auto GetId() const -> uint32_t;
		SH_USER_API auto GetHp() const -> uint32_t;
		SH_USER_API auto GetMaxHp() const -> int32_t;
		SH_USER_API auto GetSpeed() const -> float;
		SH_USER_API auto GetAttack() const->uint32_t;
		SH_USER_API auto GetLevel() const -> uint32_t;
		SH_USER_API auto GetExp() const -> uint32_t;
		SH_USER_API auto IsSpawner() const -> bool;
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

		PROPERTY(anim)
		MobAnimation* anim = nullptr;
#else
		uint64_t seq = 1;
#endif
		PROPERTY(clone, core::PropertyOption::invisible)
		Prefab* clone = nullptr;

		uint32_t hp = 15;

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