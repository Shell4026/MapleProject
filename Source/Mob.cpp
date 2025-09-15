#include "Mob.h"
#include "MapleServer.h"
#include "MapleClient.h"
#include "Skill.h"
#include "Packet/MobStatePacket.hpp"
#include "Packet/MobSpawnPacket.hpp"
#include "Packet/PlayerJoinWorldPacket.h"
#include "Packet/MobHitPacket.h"

#include "Game/GameObject.h"

#include "Network/StringPacket.h"

#include "CollisionTag.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/norm.hpp"

namespace sh::game
{
	Mob::Mob(GameObject& owner) :
		NetworkComponent(owner), spawnerUUID(core::UUID::GenerateEmptyUUID())
	{
#if !SH_SERVER
		packetSubscriber.SetCallback
		(
			[&](const PacketEvent& evt)
			{
				if (evt.packet->GetId() == MobSpawnPacket::ID)
					ProcessMobSpawn(static_cast<const MobSpawnPacket&>(*evt.packet));
				else if (evt.packet->GetId() == MobStatePacket::ID)
					ProcessState(static_cast<const MobStatePacket&>(*evt.packet));
				else if (evt.packet->GetId() == MobHitPacket::ID)
					ProcessHit(static_cast<const MobHitPacket&>(*evt.packet));

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
		if (core::IsValid(ai) && !bStun)
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
			packet.bStun = bStun;
			if (ai != nullptr)
				packet.state = ai->GetState();

			MapleServer::GetInstance()->BroadCast(packet);

			tick = 0;
		}
		if (bStun)
		{
			stunCount -= world.deltaTime * 1000;
			if (stunCount <= 0)
				bStun = false;
		}
#else
		const auto& pos = gameObject.transform->GetWorldPosition();
		glm::vec2 corrected = serverPos;
		glm::vec2 correctedVel = serverVel;
		float difSqr = glm::distance2(glm::vec2{ pos.x, pos.y }, serverPos);
		if (difSqr < 1.0f * 1.0f)
		{
			corrected = glm::mix(glm::vec2{ pos }, serverPos, 0.1f);

			if (core::IsValid(anim))
			{
				if (!bStun && !anim->IsLock())
				{
					float dx = serverPos.x - corrected.x;
					if (dx > 0.01f)
						anim->bRight = true;
					else if (dx < -0.01f)
						anim->bRight = false;

					if (std::abs(serverVel.x) < 0.01f)
						anim->SetPose(MobAnimation::Pose::Idle);
					else
						anim->SetPose(MobAnimation::Pose::Move);
				}
			}
			if (core::IsValid(rigidbody))
				correctedVel = glm::mix(glm::vec2{ rigidbody->GetLinearVelocity() }, serverVel, 1.0f);
		}
		gameObject.transform->SetWorldPosition({ corrected.x, corrected.y, 0.01f });
		gameObject.transform->UpdateMatrix();
		
		if (core::IsValid(rigidbody))
		{
			rigidbody->SetLinearVelocity({ correctedVel.x, correctedVel.y, 0.f });
			rigidbody->ResetPhysicsTransform();
		}
#endif
	}
#if !SH_SERVER
	SH_USER_API void Mob::SetAnimation(MobAnimation& anim)
	{
		this->anim = &anim;
	}
	SH_USER_API auto Mob::GetAnimation() const -> MobAnimation*
	{
		return anim;
	}
#else
	SH_USER_API void Mob::Hit(Skill& skill, Player& player)
	{
		SH_INFO_FORMAT("hit from {}", player.GetUserUUID().ToString());

		MobHitPacket packet{};
		packet.skillId = skill.GetId();
		packet.mobUUID = GetUUID().ToString(); // 몹은 모든 플레이어에게 UUID는 똑같게 보임
		packet.userUUID = player.GetUserUUID().ToString();

		MapleServer::GetInstance()->BroadCast(packet);

		bStun = true;
		stunCount = 1000;

		const auto& playerPos = player.gameObject.transform->GetWorldPosition();
		const auto& mobPos = gameObject.transform->GetWorldPosition();
		float dx = (mobPos.x - playerPos.x) < 0 ? -1.f : 1.f;

		auto v = rigidbody->GetLinearVelocity();
		rigidbody->SetLinearVelocity({ 0.f, v.y, v.z });
		rigidbody->AddForce({ dx * 100.f, 0.f, 0.f });
		
	}
#endif
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
	SH_USER_API auto Mob::IsSpawner() const -> bool
	{
		return bSpawner;
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
			bool wasStun = bStun;
			bStun = packet.bStun;

			if (wasStun && !bStun && anim->GetPose() == MobAnimation::Pose::Hit)
			{
				anim->SetLock(false);
				if (std::abs(serverVel.x) < 0.01f)
					anim->SetPose(MobAnimation::Pose::Idle);
				else
					anim->SetPose(MobAnimation::Pose::Move);
			}
		}
	}
	void Mob::ProcessHit(const MobHitPacket& packet)
	{
#if !SH_SERVER
		if (packet.mobUUID == GetUUID().ToString())
		{
			anim->SetPose(MobAnimation::Pose::Hit);
			anim->SetLock(true);
			bStun = true;
		}
#endif
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