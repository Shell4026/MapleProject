#include "Mob/MobAnimationController.h"

namespace sh::game
{
	SH_USER_API void MobAnimationController::SetAnimation(MobAnimation* anim)
	{
		this->anim = anim;
	}
    SH_USER_API void MobAnimationController::Update(const MobStatus& status, const glm::vec2& vel, float dt)
	{
        if (!anim.IsValid())
            return;
        auto pose = DecidePose(status, vel);
        anim->SetPose(pose);
	}
    SH_USER_API void MobAnimationController::SetFacingFromVelocity(glm::vec2 vel)
    {
        if (!anim.IsValid())
            return;
        if (vel.x > 0.01f) 
            anim->bRight = true;
        else if (vel.x < -0.01f) 
            anim->bRight = false;
    }
    auto MobAnimationController::DecidePose(const MobStatus& status, const glm::vec2& vel) -> MobAnimation::Pose
    {
        if (status.bStun)
            return MobAnimation::Pose::Hit;
        if (std::abs(vel.x) < 0.01f) 
            return MobAnimation::Pose::Idle;
        return MobAnimation::Pose::Move;
    }
}//namespace