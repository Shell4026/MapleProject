#include "Player/PlayerAnimator.h"
#include "Player/PlayerMovement.h"
#include "Skill/Skill.h"
#include "Skill/SkillManager.h"

#include "Game/GameObject.h"
namespace sh::game
{
	PlayerAnimator::PlayerAnimator(GameObject& owner) :
		Animator(owner)
	{
	}
	SH_USER_API void PlayerAnimator::Awake()
	{
		Super::Awake();
		if (movement == nullptr)
			SH_ERROR("movement is nullptr!");
		if (skillManager == nullptr)
			SH_ERROR("skillManager is nullptr!");
	}
	SH_USER_API void PlayerAnimator::Update()
	{
		DecideState();
		Super::Update();
		ApplyRight();
	}
	void PlayerAnimator::DecideState()
	{
		auto skillState = skillManager->GetLastSkillState();
		if (skillState != nullptr)
		{
			const Skill* const skill = skillState->skill;
			if (core::IsValid(skill))
			{
				SetState(skill->GetAnimState());
				return;
			}
		}

		if (movement->IsProne())
		{
			SetState(3);
			return;
		}

		if (!movement->IsGround())
		{
			SetState(2);
			return;
		}

		const auto& vel = movement->GetVelocity();
		if (std::abs(vel.x) > 0.1f)
			SetState(1);
		else
			SetState(0);
	}
	void PlayerAnimator::ApplyRight()
	{
		const bool bRight = movement->IsRight();

		auto scale = renderer->gameObject.transform->scale;
		if (bRight && scale.x > 0 || !bRight && scale.x < 0)
			scale.x = -scale.x;
		renderer->gameObject.transform->SetScale(scale);

		if (bRight)
		{
			if (AnimationData* curAnim = GetCurAnimation(); core::IsValid(curAnim))
			{
				const auto& animPos = curAnim->GetPos();
				renderer->gameObject.transform->SetPosition(-animPos.x, animPos.y, renderer->gameObject.transform->position.z);
			}
		}
		renderer->gameObject.transform->UpdateMatrix();
	}
}//namespace