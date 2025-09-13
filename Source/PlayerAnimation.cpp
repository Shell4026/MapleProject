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
			auto propBlock = std::make_unique<render::MaterialPropertyBlock>();
			if (idles.size() > 0)
				propBlock->SetProperty("tex", idles.front());

			meshRenderer->SetMaterialPropertyBlock(std::move(propBlock));

			initPos = gameObject.transform->position;
			initScale = gameObject.transform->scale;
		}
#endif
	}
	SH_USER_API void PlayerAnimation::Update()
	{
#if !SH_SERVER
		if (!core::IsValid(meshRenderer))
			return;

		float dir = bRight ? -1.0f : 1.0f;
		auto pos = initPos;
		auto scale = initScale;
		scale.x *= dir;
		
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
			pos.x += walkOffset.x * dir;
			pos.y += walkOffset.y;
			scale.x *= walkScale.x;
			scale.y *= walkScale.y;
			ChangeTexture(walkDelayMs, walks);
			break;
		}
		case Pose::Jump:
		{
			pos.x += jumpOffset.x * dir;
			pos.y += jumpOffset.y;
			scale.x *= jumpScale.x;
			scale.y *= jumpScale.y;
			ChangeTexture(jumpDelayMs, jumps);
			break;
		}
		case Pose::Prone:
		{
			pos.x += proneOffset.x * dir;
			pos.y += proneOffset.y;
			scale.x *= proneScale.x;
			scale.y *= proneScale.y;
			ChangeTexture(proneDelayMs, prones);
			break;
		}
		}
		gameObject.transform->SetPosition(pos);
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
	SH_USER_API void PlayerAnimation::SetMeshRenderer(MeshRenderer& meshRenderer)
	{
		this->meshRenderer = &meshRenderer;
	}
	SH_USER_API auto PlayerAnimation::GetMeshRenderer() const -> MeshRenderer*
	{
		return meshRenderer;
	}
	void PlayerAnimation::ChangeTexture(float delayMs, const std::vector<render::Texture*>& texs)
	{
		if (t >= delayMs / 1000.f)
		{
			t = 0.0f;
			texIdx = (texIdx + 1) % texs.size();
		}
		if (texs.size() > texIdx)
		{
			meshRenderer->GetMaterialPropertyBlock()->SetProperty("tex", texs[texIdx]);
		}
	}
}//namespace