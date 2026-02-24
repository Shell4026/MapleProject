#include "Mob/MobAnimatorController.h"
#include "Mob/Mob.h"
#include "Mob/MobMovement.h"
#include "Animator.h"

namespace sh::game
{
    MobAnimatorController::MobAnimatorController(GameObject& owner) :
        Component(owner)
    {
    }
    SH_USER_API void MobAnimatorController::Awake()
    {
        if (mob == nullptr)
            SH_INFO("mob is nullptr!");
        if (animator == nullptr)
            SH_INFO("animator is nullptr!");
    }
    SH_USER_API void MobAnimatorController::Update()
	{
        if (!core::IsValid(animator))
            return;
        const int state = DecideState();
        animator->SetState(state);

        ApplyRight();
	}
    auto MobAnimatorController::DecideState() const -> int
    {
        const auto vel = mob->GetMovement()->GetVelocity();
        if (std::abs(vel.x) > 0.1f)
            return 1;
        return 0;
    }
    void MobAnimatorController::ApplyRight()
    {
        const auto vel = mob->GetMovement()->GetVelocity();
        if (bRight && vel.x < -0.1f)
            bRight = false;
        if (!bRight && vel.x > 0.1f)
            bRight = true;

        auto renderer = animator->GetMeshRenderer();
        auto scale = renderer->gameObject.transform->scale;
        if (bRight && scale.x > 0 || !bRight && scale.x < 0)
            scale.x = -scale.x;
        renderer->gameObject.transform->SetScale(scale);

        if (bRight)
        {
            if (AnimationData* curAnim = animator->GetCurAnimation(); core::IsValid(curAnim))
            {
                const auto& animPos = curAnim->GetPos();
                renderer->gameObject.transform->SetPosition(-animPos.x, animPos.y, renderer->gameObject.transform->position.z);
            }
        }
    }
}//namespace