#pragma once
#include "../Export.h"
#include "../PacketEvent.hpp"
#include "../Skill.h"
#include "../AI/AIStrategy.h"
#include "../UI/HPUI.h"
#include "MobAnimation.h"
#include "MobAnimationController.h"
#include "MobStatus.hpp"

#include "Core/SContainer.hpp"
#include "Core/EventSubscriber.h"

#include "Game/Component/NetworkComponent.h"
#include "Game/Component/RigidBody.h"
#include "Game/Prefab.h"

namespace sh::game
{
    class MobStatePacket;

    class Mob : public NetworkComponent
    {
        COMPONENT(Mob, "user")
    public:
        SH_USER_API Mob(GameObject& owner);

        SH_USER_API void Awake() override;
        SH_USER_API void Update() override;

#if !SH_SERVER
        SH_USER_API void SetAnimation(MobAnimation& anim);
#endif

#if SH_SERVER
        SH_USER_API void Hit(Skill& skill, Player& player);
#endif
        SH_USER_API void SetAIStrategy(AIStrategy* strategy);

        SH_USER_API auto GetRigidbody() const -> RigidBody* { return rigidbody; }
        SH_USER_API auto GetStatus() const -> const MobStatus& { return status; }
        SH_USER_API auto GetStatus() -> MobStatus& { return status; }
        SH_USER_API auto GetSpeed() const -> float { return speed; }

        SH_USER_API void SetSpawnerUUID(const core::UUID& uuid) { spawnerUUID = uuid; }
    private:
        void ProcessState(const MobStatePacket& packet);
    protected:
        PROPERTY(maxHp)
        uint32_t maxHp = 10;
        PROPERTY(speed)
        float speed = 0.6f;

        PROPERTY(healthBar)
        HPUI* healthBar = nullptr;

        PROPERTY(ai)
        AIStrategy* ai = nullptr;

        PROPERTY(rigidbody)
        RigidBody* rigidbody = nullptr;
    private:
        MobStatus status;

        core::UUID spawnerUUID;
        uint32_t netSeq = 0;

        core::EventSubscriber<PacketEvent> packetSubscriber;
#if SH_SERVER
        float netAccum = 0.f;
        uint32_t snapshotSeq = 1;
#else
        glm::vec2 serverPos{};
        glm::vec2 serverVel{};
        MobAnimationController animator;
#endif
        PROPERTY(anim)
        MobAnimation* anim = nullptr;
    };
}//namespace