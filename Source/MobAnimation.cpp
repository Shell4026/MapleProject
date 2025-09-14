#include "MobAnimation.h"
#if !SH_SERVER
#include "Game/GameObject.h"
namespace sh::game
{
	MobAnimation::MobAnimation(GameObject& owner) :
		Component(owner)
	{
	}
	SH_USER_API void MobAnimation::Awake()
	{
		if (meshRenderer == nullptr)
			return;
		if (idle != nullptr)
		{
			idle->SetTarget(*gameObject.transform);
			idle->Play(*meshRenderer);
		}
		if (move != nullptr)
			move->SetTarget(*gameObject.transform);
		if (jump != nullptr)
			jump->SetTarget(*gameObject.transform);
	}
	SH_USER_API void MobAnimation::BeginUpdate()
	{
#if !SH_SERVER
		if (curAnim.IsValid())
			curAnim->InverseX(bRight);
#endif
	}
	SH_USER_API void MobAnimation::SetPose(Pose pose)
	{
		if (curPose == pose)
			return;
		curPose = pose;
		if (!core::IsValid(meshRenderer))
			return;
		if (curAnim.IsValid())
			curAnim->Stop();
		curAnim.Reset();

		switch (pose)
		{
		case Pose::Idle:
			if (core::IsValid(idle))
			{
				curAnim = idle;
				idle->Play(*meshRenderer);
			}
			break;
		case Pose::Move:
			if (core::IsValid(move))
			{
				curAnim = move;
				move->Play(*meshRenderer);
			}
			break;
		case Pose::Jump:
			if (core::IsValid(jump))
			{
				curAnim = jump;
				jump->Play(*meshRenderer);
			}
			break;
		}
		curAnim->InverseX(bRight);
	}
	SH_USER_API void MobAnimation::SetMeshRenderer(MeshRenderer& meshRenderer)
	{
		this->meshRenderer = &meshRenderer;
	}
	SH_USER_API auto MobAnimation::GetMeshRenderer() const -> MeshRenderer*
	{
		return meshRenderer;
	}
}//namespace
#endif
