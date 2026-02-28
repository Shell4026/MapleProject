#include "System/Animation.h"
#include "Game/World.h"
namespace sh::game
{
	Animation::Animation(GameObject& owner) :
		Component(owner)
	{
	}
	SH_USER_API void Animation::SetTarget(Transform& transform)
	{
		target = &transform;

		initPos = transform.position;
		initScale = transform.scale;
	}
	SH_USER_API void Animation::InverseX(bool bInverse)
	{
		bRight = bInverse;
	}
	SH_USER_API void Animation::Play(MeshRenderer& meshRenderer)
	{
		bPlaying = true;
		this->meshRenderer = &meshRenderer;
		if (meshRenderer.GetMaterialPropertyBlock() == nullptr)
		{
			auto propBlock = std::make_unique<render::MaterialPropertyBlock>();
			meshRenderer.SetMaterialPropertyBlock(std::move(propBlock));
		}
		idx = 0;
		t = 0.0f;
	}
	SH_USER_API void Animation::Stop()
	{
		idx = 0;
		t = 0.0f;
		bPlaying = false;
		if (target.IsValid())
		{
			target->SetPosition(initPos);
			target->SetScale(initScale);
		}
	}
	SH_USER_API void Animation::SetLoop(bool bLoop)
	{
		this->bLoop = bLoop;
	}
	SH_USER_API auto Animation::IsLoop() const -> bool
	{
		return bLoop;
	}
	SH_USER_API auto Animation::IsPlaying() const -> bool
	{
		return bPlaying;
	}
	SH_USER_API auto Animation::GetCurrentTexture() const -> render::Texture*
	{
		if (textures.empty())
			return nullptr;
		if (idx < textures.size())
			return textures[idx];
		return nullptr;
	}
	SH_USER_API void Animation::Update()
	{
		if (!bPlaying)
			return;
		if (textures.empty())
			return;

		uint32_t delay = 1000;
		if (!delays.empty())
		{
			if (delays.size() > idx)
				delay = delays[idx];
			else
				delay = delays.back();
		}

		if (t >= delay / 1000.f)
		{
			if (bLoop)
			{
				idx = (idx + 1) % textures.size();
				if (delays.size() > idx)
					delay = delays[idx];
			}
			else
			{
				idx = (idx + 1);
				if (idx >= textures.size())
				{
					idx = textures.size() - 1;
					bPlaying = false;
				}
			}
			t = 0.0f;
		}
		if (meshRenderer.IsValid())
		{
			meshRenderer->GetMaterialPropertyBlock()->SetProperty("tex", textures[idx]);
			meshRenderer->UpdatePropertyBlockData();
		}
		
		if (target.IsValid())
		{
			auto pos = initPos;
			pos.x += bRight ? -offset.x : offset.x;
			pos.y += offset.y;

			target->SetPosition(pos);
			target->SetScale({ bRight ? -scale.x : scale.x, scale.y, initScale.z });
			target->UpdateMatrix();
		}
		t += world.deltaTime;
	}
}//namespace