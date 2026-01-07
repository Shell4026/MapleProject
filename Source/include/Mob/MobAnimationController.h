#pragma once
#include "../Export.h"
#include "MobAnimation.h"
#include "MobStatus.hpp"

#include "glm/vec2.hpp"

#include <cstdint>

#include "Core/SContainer.hpp"
namespace sh::game
{
    class MobAnimationController
    {
    public:
        SH_USER_API void SetAnimation(MobAnimation* anim);

        SH_USER_API void Update(const MobStatus& status, const glm::vec2& vel, float dt);

        SH_USER_API void SetFacingFromVelocity(glm::vec2 vel);
    private:
        static auto DecidePose(const MobStatus& status, const glm::vec2& vel) -> MobAnimation::Pose;
    private:
        core::SObjWeakPtr<MobAnimation> anim = nullptr;
        bool bHitOverride = false;
    };
}