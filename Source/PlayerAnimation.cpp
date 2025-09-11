#include "PlayerAnimation.h"

#include "Game/World.h"
#include "Game/GameObject.h"

#include "Render/Material.h"
namespace sh::game
{
	PlayerAnimation::PlayerAnimation(GameObject& owner) :
		Component(owner)
	{
	}
	SH_USER_API void PlayerAnimation::Awake()
	{
#if !SH_SERVER
		if (core::IsValid(meshRenderer))
		{
			auto oldMatPtr = meshRenderer->GetMaterial();
			if (!core::IsValid(oldMatPtr))
				return;

			mat = core::SObject::Create<render::Material>(*oldMatPtr);
			meshRenderer->SetMaterial(mat.Get());

			initPos = meshRenderer->gameObject.transform->position;
			initScale = gameObject.transform->scale;
		}
#endif
	}
	SH_USER_API void PlayerAnimation::Update()
	{
#if !SH_SERVER
		if (!mat.IsValid())
			return;

		auto scale = initScale;
		scale.x *= bRight ? -1.f : 1.f;

		switch (curPose)
		{
		case Pose::Idle:
		{
			scale.x *= idleScale.x;
			scale.y *= idleScale.y;
			ChangeTexture(idleDelayMs, idles);
			break;
		}
		case Pose::Walk:
		{
			scale.x *= walkScale.x;
			scale.y *= walkScale.y;
			ChangeTexture(walkDelayMs, walks);
			break;
		}
		case Pose::Jump:
		{
			scale.x *= jumpScale.x;
			scale.y *= jumpScale.y;
			ChangeTexture(jumpDelayMs, jumps);
			break;
		}
		}
		gameObject.transform->SetScale(scale);

		t += world.deltaTime;
#endif
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