#pragma once
#include "Export.h"
#include "Skill/Projectile.h"
#include "MapleWorld.h"
#include "MobStatus.hpp"

#include "AI/AIStrategy.h"

#include "Network/PacketEvent.hpp"

#if !SH_SERVER
#include "UI/HPUI.h"
#endif

#include "Core/EventSubscriber.h"
#include "Core/EventBus.h"

#include "Game/Component/Component.h"
#include "Game/Component/Phys/RigidBody.h"

namespace sh::game
{
    class MobMovement;
    class MobStatePacket;
    class ItemDropPacket;

    class Mob : public Component
    {
        COMPONENT(Mob, "user")
    public:
        SH_USER_API Mob(GameObject& owner);

        SH_USER_API void Awake() override;
        SH_USER_API void Update() override;
        SH_USER_API void OnTriggerEnter(Collider& collider) override;

        SH_USER_API void Reset();
        SH_USER_API void SetAIStrategy(AIStrategy* strategy) { ai = strategy; }
#if SH_SERVER
        SH_USER_API void Kill(const Player& player);
        SH_USER_API void BroadcastStatePacket();
        SH_USER_API void Hit(const Projectile& projectile, Player& player);
#endif

        SH_USER_API auto GetMaxHP() const -> uint32_t { return maxHp; }
        SH_USER_API auto GetMovement() const -> MobMovement* { return movement; }
        SH_USER_API auto GetStatus() const -> const MobStatus& { return status; }
        SH_USER_API auto GetStatus() -> MobStatus& { return status; }
        SH_USER_API auto GetMapleWorld() const -> MapleWorld* { return mapleWorld; }
    private:
#if !SH_SERVER
        void ProcessState(const MobStatePacket& packet);
#endif
    public:
        core::EventBus evtBus;
    protected:
        PROPERTY(maxHp)
        uint32_t maxHp = 10;
        PROPERTY(ai)
        AIStrategy* ai = nullptr;
        PROPERTY(movement, core::PropertyOption::sobjPtr)
        MobMovement* movement = nullptr;

#if !SH_SERVER
        PROPERTY(healthBar)
        HPUI* healthBar = nullptr;
#endif
    private:
        PROPERTY(mapleWorld)
        MapleWorld* mapleWorld = nullptr;
        PROPERTY(rigidbody)
        RigidBody* rigidbody = nullptr;
        PROPERTY(mobId)
        int mobId = 0;
        MobStatus status;

        uint32_t netSeq = 0;

        core::EventSubscriber<network::PacketEvent> packetSubscriber;

        game::Vec3 initPos;
#if SH_SERVER
        float netAccum = 0.f;
        uint32_t snapshotSeq = 1;
#else
        glm::vec2 serverPos{};
        glm::vec2 serverVel{};
#endif
    };
}//namespace