#include "Mob.h"
#include "MobEvents.hpp"
#include "../MapleServer.h"
#include "../MapleClient.h"
#include "../Packet/MobStatePacket.hpp"
#include "../CollisionTag.hpp"

#include "Game/World.h"
#include "Game/GameObject.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/norm.hpp"

namespace sh::game
{
    Mob::Mob(GameObject& owner) : 
        NetworkComponent(owner)
    {
        packetSubscriber.SetCallback(
            [&](const PacketEvent& evt)
            {
#if !SH_SERVER
                if (evt.packet->GetId() == MobStatePacket::ID)
                    ProcessState(static_cast<const MobStatePacket&>(*evt.packet));
#endif
            }
        );
    }

    void Mob::Awake()
    {
        SetPriority(-1);
        status.Reset(maxHp);

        if (core::IsValid(rigidbody))
        {
            rigidbody->GetCollider()->SetCollisionTag(tag::entityTag);
            rigidbody->GetCollider()->SetAllowCollisions(tag::groundTag);
        }

        gameObject.SetActive(false);
#if SH_SERVER
        initPos = gameObject.transform->GetWorldPosition();
        MapleServer::GetInstance()->bus.Subscribe(packetSubscriber);
#else
        MapleClient::GetInstance()->bus.Subscribe(packetSubscriber);
        animator.SetAnimation(anim);
#endif
    }

    void Mob::Update()
    {
        status.Tick(world.deltaTime);

#if SH_SERVER
        if (core::IsValid(ai) && !status.bStun)
            ai->Run(*this);

        // 1초에 10번 전송
        netAccum += world.deltaTime;
        if (netAccum >= 0.1f)
        {
            netAccum -= 0.1f;

            BroadcastStatePacket();
        }
#else
        // 클라 보정
        const auto& pos = gameObject.transform->GetWorldPosition();
        glm::vec2 curPos{ pos.x, pos.y };
        glm::vec2 curVel{};
        if (core::IsValid(rigidbody))
        {
            auto v = rigidbody->GetLinearVelocity();
            curVel = { v.x, v.y };
        }

        glm::vec2 correctedPos, correctedVel;

        float d2 = glm::distance2(curPos, serverPos);
        const float snapDistance = 1.0f;
        // 너무 멀면 즉시 서버 위치로
        if (d2 > snapDistance * snapDistance)
        {
            correctedPos = serverPos;
            correctedVel = serverVel;
        }
        else
        {
            correctedPos = glm::mix(curPos, serverPos, 0.1f);
            correctedVel = glm::mix(curVel, serverVel, 0.1f);
        }

        gameObject.transform->SetWorldPosition({ correctedPos.x, correctedPos.y, 0.01f });
        gameObject.transform->UpdateMatrix();

        if (core::IsValid(rigidbody))
        {
            rigidbody->SetLinearVelocity({ correctedVel.x, correctedVel.y, 0.f });
            rigidbody->ResetPhysicsTransform();
        }

        // 애니메이션
        if (core::IsValid(anim))
        {
            animator.SetFacingFromVelocity(serverVel);
            animator.Update(status, serverVel, world.deltaTime);
        }
#endif
    }

#if !SH_SERVER
    SH_USER_API void Mob::SetAnimation(MobAnimation& anim)
    {
        this->anim = &anim;
        animator.SetAnimation(this->anim);
    }
#else
    void Mob::Hit(Skill& skill, Player& player)
    {
        status.ApplyDamage(skill.GetDamage());
        status.ApplyStun(1000.f);

        if (status.hp == 0)
        {
            MobDeathEvent evt{ *this };

            evtBus.Publish(evt);
            BroadcastStatePacket();
            gameObject.SetActive(false);
            return;
        }

        // 넉백
        const auto& playerPos = player.gameObject.transform->GetWorldPosition();
        const auto& mobPos = gameObject.transform->GetWorldPosition();
        float dx = (mobPos.x - playerPos.x) < 0 ? -1.f : 1.f;

        if (core::IsValid(rigidbody))
        {
            auto v = rigidbody->GetLinearVelocity();
            rigidbody->SetLinearVelocity({ 0.f, v.y, v.z });
            rigidbody->AddForce({ dx * 100.f, 0.f, 0.f });
        }

        netAccum = 0.1f; // 즉시 state 패킷 전송
    }
    SH_USER_API void Mob::BroadcastStatePacket()
    {
        const auto& pos = gameObject.transform->GetWorldPosition();
        game::Vec3 vel{};
        if (core::IsValid(rigidbody))
            vel = rigidbody->GetLinearVelocity();

        MobStatePacket packet{};
        packet.mobUUID = GetUUID().ToString();

        packet.x = pos.x; packet.y = pos.y;
        packet.vx = vel.x; packet.vy = vel.y;

        packet.hp = status.hp;
        packet.seq = snapshotSeq++;

        packet.bStun = status.bStun;
        packet.stunRemainingMs = static_cast<uint32_t>(status.stunRemainingMs);

        if (ai != nullptr)
            packet.state = ai->GetState();

        MapleServer::GetInstance()->BroadCast(packet);
    }
#endif

    SH_USER_API void Mob::Reset()
    {
#if SH_SERVER
        netAccum = 0.f;
#else
        serverPos = { initPos.x, initPos.y };
        serverVel = { 0.f, 0.f };
#endif
        gameObject.transform->SetWorldPosition(initPos);
        gameObject.transform->UpdateMatrix();
        if (core::IsValid(rigidbody))
        {
            rigidbody->SetLinearVelocity({ 0.f, 0.f, 0.f });
            rigidbody->ResetPhysicsTransform();
        }
        if (ai != nullptr)
            ai->Reset();
    }

    void Mob::SetAIStrategy(AIStrategy* strategy)
    {
        ai = strategy;
    }

#if !SH_SERVER
    void Mob::ProcessState(const MobStatePacket& packet)
    {
        if (packet.mobUUID != GetUUID().ToString())
            return;
        if (netSeq >= packet.seq)
            return;

        serverPos = { packet.x, packet.y };
        serverVel = { packet.vx, packet.vy };

        status.hp = packet.hp;
        status.bStun = packet.bStun;
        status.stunRemainingMs = static_cast<float>(packet.stunRemainingMs);

        netSeq = packet.seq;

        if (status.hp == 0)
        {
            gameObject.SetActive(false);
        }

        if (healthBar)
        {
            healthBar->SetMaxHp(status.maxHp);
            healthBar->SetHp(status.hp);
        }
    }
#endif
}
