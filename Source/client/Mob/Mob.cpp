#include "Mob/Mob.h"
#include "Mob/MobMovement.h"

#include "World/MapleClient.h"
#include "Phys/CollisionTag.hpp"
#include "Packet/MobStatePacket.hpp"

#include "Game/World.h"
#include "Game/GameObject.h"
#include "Game/Component/Phys/Collider.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/norm.hpp"

// 클라
namespace sh::game
{
    Mob::Mob(GameObject& owner) : 
        Component(owner)
    {
        packetSubscriber.SetCallback(
            [&](const network::PacketEvent& evt)
            {
                if (evt.packet->GetId() == MobStatePacket::ID)
                    ProcessState(static_cast<const MobStatePacket&>(*evt.packet));
            }
        );
    }

    SH_USER_API void Mob::Awake()
    {
        if (rigidbody == nullptr)
            SH_INFO("rigidbody is nullptr!");
        SetPriority(-1);
        status.Reset(maxHp);

        gameObject.SetActive(false);

        MapleClient::GetInstance()->bus.Subscribe(packetSubscriber);
        initPos = gameObject.transform->GetWorldPosition();
    }

    void Mob::Update()
    {
        const auto& pos = gameObject.transform->GetWorldPosition();
        status.Tick(world.deltaTime);

        // 클라 보정
        glm::vec2 curPos{ pos.x, pos.y };
        glm::vec2 curVel{};
        if (core::IsValid(movement))
        {
            const auto v = movement->GetVelocity();
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

        if (core::IsValid(movement))
            movement->SetVelocity(correctedVel.x, correctedVel.y);

        rigidbody->ResetPhysicsTransform();
    }

    SH_USER_API void Mob::OnTriggerEnter(Collider& collider)
    {
    }

    SH_USER_API void Mob::Reset()
    {
        serverPos = { initPos.x, initPos.y };
        serverVel = { 0.f, 0.f };

        gameObject.transform->SetWorldPosition(initPos);
        gameObject.transform->UpdateMatrix();
        if (core::IsValid(movement))
            movement->SetVelocity(0.f, 0.f);
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
