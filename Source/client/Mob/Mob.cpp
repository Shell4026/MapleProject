#include "Mob/Mob.h"

#include "MapleClient.h"
#include "CollisionTag.hpp"
#include "Packet/MobStatePacket.hpp"

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
                if (evt.packet->GetId() == MobStatePacket::ID)
                    ProcessState(static_cast<const MobStatePacket&>(*evt.packet));
            }
        );
    }

    SH_USER_API void Mob::Awake()
    {
        SetPriority(-1);
        status.Reset(maxHp);

        if (core::IsValid(rigidbody))
        {
            rigidbody->GetCollider()->SetCollisionTag(tag::entityTag);
            rigidbody->GetCollider()->SetAllowCollisions(tag::groundTag);
        }
        gameObject.SetActive(false);

        MapleClient::GetInstance()->bus.Subscribe(packetSubscriber);
        animator.SetAnimation(anim);
        initPos = gameObject.transform->GetWorldPosition();
    }

    void Mob::Update()
    {
        const auto& pos = gameObject.transform->GetWorldPosition();
        status.Tick(world.deltaTime);

        // 클라 보정
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
        }
        else
        {
            correctedPos = glm::mix(curPos, serverPos, 0.1f);
        }
        correctedVel = serverVel;

        gameObject.transform->SetWorldPosition({ correctedPos.x, correctedPos.y, pos.z });
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
    }

    SH_USER_API void Mob::SetAnimation(MobAnimation& anim)
    {
        this->anim = &anim;
        animator.SetAnimation(this->anim);
    }

    SH_USER_API void Mob::Reset()
    {
        serverPos = { initPos.x, initPos.y };
        serverVel = { 0.f, 0.f };

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

    void Mob::ProcessState(const MobStatePacket& packet)
    {
        if (GetUUID() != packet.mobUUID)
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
}
