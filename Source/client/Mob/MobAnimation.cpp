#include "Mob/MobAnimation.h"
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
			curAnim = idle;
		}
		if (move != nullptr)
			move->SetTarget(*gameObject.transform);
		if (jump != nullptr)
			jump->SetTarget(*gameObject.transform);
		if (hit != nullptr)
			hit->SetTarget(*gameObject.transform);
	}
	SH_USER_API void MobAnimation::BeginUpdate()
	{
		if (curAnim.IsValid())
			curAnim->InverseX(bRight);
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
				curAnim->Play(*meshRenderer);
			}
			break;
		case Pose::Move:
			if (core::IsValid(move))
			{
				curAnim = move;
				curAnim->Play(*meshRenderer);
			}
			break;
		case Pose::Jump:
			if (core::IsValid(jump))
			{
				curAnim = jump;
				curAnim->Play(*meshRenderer);
			}
			break;
		case Pose::Hit:
			if (core::IsValid(hit))
			{
				curAnim = hit;
				curAnim->Play(*meshRenderer);
			}
			break;
		}
		curAnim->InverseX(bRight);
	}
	SH_USER_API auto MobAnimation::GetPose() const -> Pose
	{
		return curPose;
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