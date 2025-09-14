#include "PlayerAnimation.h"
#if !SH_SERVER
#include "Game/GameObject.h"
namespace sh::game
{
	PlayerAnimation::PlayerAnimation(GameObject& owner) :
		Component(owner)
	{
	}
	SH_USER_API void PlayerAnimation::Awake()
	{
		if (meshRenderer == nullptr)
			return;
		if (idle != nullptr)
		{
			idle->SetTarget(*gameObject.transform);
			idle->Play(*meshRenderer);
		}
		if (walk != nullptr)
			walk->SetTarget(*gameObject.transform);
		if (jump != nullptr)
			jump->SetTarget(*gameObject.transform);
		if (prone != nullptr)
			prone->SetTarget(*gameObject.transform);
	}
	SH_USER_API void PlayerAnimation::BeginUpdate()
	{
#if !SH_SERVER
		if (curAnim.IsValid())
			curAnim->InverseX(bRight);
#endif
	}
	SH_USER_API void PlayerAnimation::SetPose(Pose pose)
	{
		if (curPose == pose)
			return;
		curPose = pose;
		if (!core::IsValid(meshRenderer))
			return;
		if (curAnim.IsValid())
			curAnim->Stop();

		switch (pose)
		{
		case Pose::Idle:
			if (core::IsValid(idle))
			{
				curAnim = idle;
				idle->Play(*meshRenderer);
			}
			break;
		case Pose::Walk:
			if (core::IsValid(walk))
			{
				curAnim = walk;
				walk->Play(*meshRenderer);
			}
			break;
		case Pose::Jump:
			if (core::IsValid(jump))
			{
				curAnim = jump;
				jump->Play(*meshRenderer);
			}
			break;
		case Pose::Prone:
			if (core::IsValid(prone))
			{
				curAnim = prone;
				prone->Play(*meshRenderer);
			}
			break;
		}
		curAnim->InverseX(bRight);
	}
	SH_USER_API void PlayerAnimation::SetMeshRenderer(MeshRenderer& meshRenderer)
	{
		this->meshRenderer = &meshRenderer;
	}
	SH_USER_API auto PlayerAnimation::GetMeshRenderer() const -> MeshRenderer*
	{
		return meshRenderer;
	}
}//namespace
#endif
