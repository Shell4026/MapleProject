#include "PlayerAnimation.h"

#include "Game/World.h"
#include "Game/GameObject.h"
namespace sh::game
{
	PlayerAnimation::PlayerAnimation(GameObject& owner) :
		Component(owner)
	{
	}
	SH_USER_API void PlayerAnimation::Start()
	{
		if (core::IsValid(meshRenderer))
		{
			mat = meshRenderer->GetMaterial();
			initPos = meshRenderer->gameObject.transform->position;
			initMeshScale = meshRenderer->gameObject.transform->scale;
		}
	}
	SH_USER_API void PlayerAnimation::Update()
	{
		if (!mat.IsValid())
			return;

		if (bRight)
			meshRenderer->gameObject.transform->SetScale(-initMeshScale.x, initMeshScale.y, initMeshScale.z);
		else
			meshRenderer->gameObject.transform->SetScale(initMeshScale.x, initMeshScale.y, initMeshScale.z);

		switch (curPose)
		{
		case Pose::Idle:
		{
			posOffset = { 0.f, 0.f, 0.f };
			ChangeTexture(500, idles);
			break;
		}
		case Pose::Walk:
		{
			ChangeTexture(180, walks);
			break;
		}
		case Pose::Jump:
		{
			ChangeTexture(1000, jumps);
			break;
		}
		}
		t += world.deltaTime;
	}
	SH_USER_API void PlayerAnimation::SetPose(Pose pose)
	{
		if (pose != curPose)
		{
			curPose = pose;
			texIdx = 0;
		}
	}
	void PlayerAnimation::ChangeTexture(float delayMs, const std::vector<render::Texture*>& texs)
	{
		if (t >= delayMs / 1000.f)
		{
			t = 0.0f;
			texIdx = (texIdx + 1) % texs.size();
		}
		if (texs.size() > texIdx)
			mat->SetProperty("tex", texs[texIdx]);
	}
}//namespace