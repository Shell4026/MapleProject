#include "Mob/Mob.h"
#include "Mob/MobEvents.hpp"
#include "Mob/MobMovement.h"
#include "MapleServer.h"
#include "CollisionTag.hpp"
#include "Item/ItemDropManager.h"

#include "Packet/MobStatePacket.hpp"

#include "Game/GameObject.h"

// 서버 사이드
namespace sh::game
{
    Mob::Mob(GameObject& owner) :
        Component(owner)
    {
    }

    SH_USER_API void Mob::Awake()
    {
        SetPriority(-1);
        status.Reset(maxHp);

        gameObject.SetActive(false);

        MapleServer::GetInstance()->bus.Subscribe(packetSubscriber);
        initPos = gameObject.transform->GetWorldPosition();
    }

    SH_USER_API void Mob::Update()
    {
        const auto& pos = gameObject.transform->GetWorldPosition();
        status.Tick(world.deltaTime);

        if (core::IsValid(ai) && !status.bStun)
            ai->Run(*this);

        // 1초에 10번 전송
        netAccum += world.deltaTime;
        if (netAccum >= 0.1f)
        {
            netAccum -= 0.1f;

            BroadcastStatePacket();
        }
    }

    SH_USER_API void Mob::Reset()
    {
        netAccum = 0.f;

        gameObject.transform->SetWorldPosition(initPos);
        gameObject.transform->UpdateMatrix();
        if (core::IsValid(movement))
            movement->SetVelocity(0.f, 0.f);

        if (ai != nullptr)
            ai->Reset();
    }

    SH_USER_API void Mob::Hit(Skill& skill, Player& player)
    {
        status.ApplyDamage(skill.GetDamage());
        status.ApplyStun(1000.f);

        if (status.hp == 0)
        {
            Kill(player);
            return;
        }

        // 넉백
        const auto& playerPos = player.gameObject.transform->GetWorldPosition();
        const auto& mobPos = gameObject.transform->GetWorldPosition();
        float dx = (mobPos.x - playerPos.x) < 0 ? -1.f : 1.f;

        if (core::IsValid(movement))
        {
            auto v = movement->GetVelocity();
            movement->SetVelocity(0.f, v.y);
            //rigidbody->AddForce({ dx * 100.f, 0.f, 0.f });
        }

        netAccum = 0.1f; // 즉시 state 패킷 전송
    }

    SH_USER_API void Mob::Kill(const Player& player)
    {
        MobDeathEvent evt{ *this };

        evtBus.Publish(evt);
        BroadcastStatePacket();
        gameObject.SetActive(false);

        std::vector<int> dropItems = ItemDropManager::GetInstance()->DropItem(mobId);
        if (!dropItems.empty())
        {
            auto& pos = gameObject.transform->GetWorldPosition();
            mapleWorld->SpawnItem(dropItems, pos.x, pos.y, player.GetUserUUID());
        }
    }

    SH_USER_API void Mob::BroadcastStatePacket()
    {
        const auto& pos = gameObject.transform->GetWorldPosition();
        Vec2 vel{};
        if (core::IsValid(movement))
            vel = movement->GetVelocity();

        MobStatePacket packet{};
        packet.mobUUID = GetUUID();

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
}//namespace