#include "Mob.h"
#include "MapleServer.h"
#include "MapleClient.h"
#include "Packet/MobStatePacket.hpp"
#include "Packet/MobSpawnPacket.hpp"
#include "Packet/PlayerJoinWorldPacket.h"

#include "Game/GameObject.h"

#include "CollisionTag.hpp"

namespace sh::game
{
	Mob::Mob(GameObject& owner) :
		NetworkComponent(owner), spawnerUUID(core::UUID::GenerateEmptyUUID())
	{
#if !SH_SERVER
		// 스포너의 경우만 구독함
		packetSubscriber.SetCallback
		(
			[&](const PacketEvent& evt)
			{
				if (evt.packet->GetId() == MobSpawnPacket::ID)
					ProcessMobSpawn(static_cast<const MobSpawnPacket&>(*evt.packet));
				else if (evt.packet->GetId() == MobStatePacket::ID)
				{
					ProcessState(static_cast<const MobStatePacket&>(*evt.packet));
				}

			}
		);
#else
		packetSubscriber.SetCallback
		(
			[&](const PacketEvent& evt)
			{
				//if (evt.packet->GetId() == PlayerJoinWorldPacket::ID)
				//	ProcessPlayerJoin(static_cast<const PlayerJoinWorldPacket&>(*evt.packet));
			}
		);
#endif
	}
	SH_USER_API void Mob::Awake()
	{
		SetPriority(-1);
		hp = maxHp;

		if (rigidbody != nullptr)
		{
			rigidbody->GetCollider()->SetCollisionTag(tag::entityTag);
			rigidbody->GetCollider()->SetAllowCollisions(tag::groundTag);
		}
		// 스포너의 경우 UUID는 서버와 클라 둘다 같음
#if SH_SERVER
		MapleServer::GetInstance()->bus.Subscribe(packetSubscriber);
		if (bSpawner)
		{
			clone = Prefab::CreatePrefab(gameObject);
			SpawnMob();
			gameObject.SetActive(false);
		}
		else
		{
			SH_INFO("Hello!");
			MobSpawnPacket packet{};
			packet.spawnerUUID = spawnerUUID.ToString();
			packet.mobUUID = GetUUID().ToString();

			MapleServer::GetInstance()->BroadCast(packet);
		}
#else
		MapleClient::GetInstance()->bus.Subscribe(packetSubscriber);
		if (bSpawner)
		{
			clone = Prefab::CreatePrefab(gameObject);
			gameObject.SetActive(false);
		}
#endif
	}
	SH_USER_API void Mob::Update()
	{
#if SH_SERVER
		if (bSpawner)
			return;
		if (core::IsValid(ai))
			ai->Run(*this);
		++tick;
		if (tick == 6) // 60fps 기준 1초에 10번
		{
			const auto& pos = gameObject.transform->GetWorldPosition();
			game::Vec3 vel{};
			if (core::IsValid(rigidbody))
				vel = rigidbody->GetLinearVelocity();

			MobStatePacket packet{};
			packet.spawnerUUID = spawnerUUID.ToString();
			packet.mobUUID = GetUUID().ToString();
			packet.x = pos.x;
			packet.y = pos.y;
			packet.vx = vel.x;
			packet.vy = vel.y;
			packet.hp = hp;
			packet.seq = seq++;
			if (ai != nullptr)
				packet.state = ai->GetState();

			MapleServer::GetInstance()->BroadCast(packet);

			tick = 0;
		}
#else
		auto pos = glm::mix(glm::vec2{ gameObject.transform->GetWorldPosition() }, serverPos, 0.1f);
		if (anim != nullptr)
		{
			float dx = serverPos.x - pos.x;
			if (dx > 0.01f)
				anim->bRight = true;
			else if (dx < -0.01f)
				anim->bRight = false;

			if (std::abs(serverVel.x) < 0.01f)
				anim->SetPose(PlayerAnimation::Pose::Idle);
			else
				anim->SetPose(PlayerAnimation::Pose::Walk);
		}
		gameObject.transform->SetWorldPosition(pos);
		gameObject.transform->UpdateMatrix();
		if (core::IsValid(rigidbody))
		{
			auto vel = glm::mix(glm::vec2{ rigidbody->GetLinearVelocity() }, serverVel, 1.0f);
			rigidbody->SetLinearVelocity(vel);
			rigidbody->ResetPhysicsTransform();
		}
#endif
	}
	SH_USER_API void Mob::SetAIStrategy(AIStrategy* strategy)
	{
		ai = strategy;
	}
	SH_USER_API auto Mob::GetAI() const -> AIStrategy*
	{
		return ai;
	}
	SH_USER_API auto Mob::GetRigidbody() const -> RigidBody*
	{
		return rigidbody;
	}
	SH_USER_API auto Mob::GetAnim() const -> PlayerAnimation*
	{
		return anim;
	}
	SH_USER_API auto Mob::GetId() const -> uint32_t
	{
		return id;
	}
	SH_USER_API auto Mob::GetHp() const -> uint32_t
	{
		return hp;
	}
	SH_USER_API auto Mob::GetMaxHp() const -> int32_t
	{
		return maxHp;
	}
	SH_USER_API auto Mob::GetSpeed() const -> float
	{
		return speed;
	}
	SH_USER_API auto Mob::GetAttack() const -> uint32_t
	{
		return attack;
	}
	SH_USER_API auto Mob::GetLevel() const -> uint32_t
	{
		return level;
	}
	SH_USER_API auto Mob::GetExp() const -> uint32_t
	{
		return exp;
	}
#if !SH_SERVER
	void Mob::ProcessMobSpawn(const MobSpawnPacket& packet)
	{
		if (!bSpawner)
			return;
		if (packet.spawnerUUID != GetUUID().ToString())
			return;

		if (clone == nullptr)
			return;

		auto cloneObj = clone->AddToWorld(world);
		auto mob = cloneObj->GetComponent<Mob>();
		mob->SetUUID(core::UUID{ packet.mobUUID });
		mob->spawnerUUID = GetUUID();
		mob->bSpawner = false;
		mob->serverPos = { gameObject.transform->GetWorldPosition() };
		
		if (core::IsValid(rigidbody))
			mob->serverVel = { rigidbody->GetLinearVelocity() };
	}
	void Mob::ProcessState(const MobStatePacket& packet)
	{
		if (bSpawner)
		{
			if (packet.spawnerUUID == GetUUID().ToString())
			{
				auto mobObj = core::SObjectManager::GetInstance()->GetSObject(core::UUID{ packet.mobUUID });
				if (!core::IsValid(mobObj)) // 몹이 없음 = 생성
				{
					MobSpawnPacket spawnPacket{};
					spawnPacket.mobUUID = packet.mobUUID;
					spawnPacket.spawnerUUID = GetUUID().ToString();
					ProcessMobSpawn(spawnPacket);
				}
			}
		}
		else
		{
			if (packet.mobUUID != GetUUID().ToString())
				return;
			if (lastStateSeq >= packet.seq) // 이전 시간대의 패킷임
				return;

			serverPos = { packet.x, packet.y };
			serverVel = { packet.vx, packet.vy };
			hp = packet.hp;
			lastStateSeq = packet.seq;
		}
	}
#else
	void Mob::SpawnMob()
	{
		if (!bSpawner)
			return;
		if (clone == nullptr)
		{
			SH_ERROR("Clone is not exist!");
			return;
		}
		auto cloneObj = clone->AddToWorld(world);
		auto mob = cloneObj->GetComponent<Mob>();
		mob->spawnerUUID = GetUUID();
		mob->bSpawner = false;
		// 스폰 패킷은 Awake에서
	}
	void Mob::ProcessPlayerJoin(const PlayerJoinWorldPacket& packet)
	{
		if (packet.worldUUID != world.GetUUID().ToString())
			return;

		MobSpawnPacket spawnPacket{};
		spawnPacket.spawnerUUID = spawnerUUID.ToString();
		spawnPacket.mobUUID = GetUUID().ToString();

		MapleServer::GetInstance()->BroadCast(spawnPacket);
	}
#endif
}//namespace