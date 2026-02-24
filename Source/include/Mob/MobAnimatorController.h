#pragma once
#include "../Export.h"
#include "MobStatus.hpp"

#include "Game/Component/Component.h"

#include "glm/vec2.hpp"

#include <cstdint>

#include "Core/SContainer.hpp"
namespace sh::game
{
    class Mob;
    class Animator;
    class MobAnimatorController : public Component
    {
        COMPONENT(MobAnimatorController, "user")
    public:
        SH_USER_API MobAnimatorController(GameObject& owner);

        SH_USER_API void Awake() override;
        SH_USER_API void Update() override;
    private:
        auto DecideState() const -> int;
        void ApplyRight();
    private:
        PROPERTY(mob, core::PropertyOption::sobjPtr)
        Mob* mob = nullptr;
        PROPERTY(animator, core::PropertyOption::sobjPtr)
        Animator* animator = nullptr;

        bool bRight = false;
    };
}