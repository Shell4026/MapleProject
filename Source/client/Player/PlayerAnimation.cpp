#include "Player/PlayerAnimation.h"
#include "Player/Player.h"

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
		{
			SH_ERROR("meshRenderer is nullptr");
			return;
		}
		if (movement == nullptr)
			SH_ERROR("movement is nullptr");
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
		DecidePose();
		if (curAnim.IsValid() && core::IsValid(player))
			curAnim->InverseX(player->IsRight());
	}
	SH_USER_API void PlayerAnimation::SetPose(Pose pose)
	{
		if (curPose == pose || bAnimLock)
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
		if (curAnim.IsValid() && core::IsValid(player))
			curAnim->InverseX(player->IsRight());
	}
	SH_USER_API void PlayerAnimation::SetMeshRenderer(MeshRenderer& meshRenderer)
	{
		this->meshRenderer = &meshRenderer;
	}
	SH_USER_API auto PlayerAnimation::GetMeshRenderer() const -> MeshRenderer*
	{
		return meshRenderer;
	}
	SH_USER_API auto PlayerAnimation::GetPlayer() const -> Player*
	{
		return player;
	}
	SH_USER_API void PlayerAnimation::SetLock(bool bLock)
	{
		bAnimLock = bLock;
	}
	SH_USER_API auto PlayerAnimation::IsLock() const -> bool
	{
		return bAnimLock;
	}
	void PlayerAnimation::DecidePose()
	{
		if (movement != nullptr)
		{
			auto vel = movement->GetVelocity();
			if (std::abs(vel.y) > 0.f)
				SetPose(Pose::Jump);
			else if (std::abs(vel.x) > 0.f)
				SetPose(Pose::Walk);
			else
				SetPose(Pose::Idle);
		}
	}
}//namespace
