#include "Animator.h"
#include "Animator.h"
#include "Animator.h"
#include "Animator.h"

#include "Game/World.h"

namespace sh::game
{
	Animator::Animator(GameObject& owner) :
		Component(owner)
	{
	}
	SH_USER_API void Animator::Awake()
	{
		if (renderer == nullptr)
			SH_ERROR("renderer is nullptr!");
		else
		{
			const auto& pos = renderer->gameObject.transform->position;
			const auto& scale = renderer->gameObject.transform->GetWorldScale();
			rendererPos.x = pos.x;
			rendererPos.y = pos.y;

			rendererScale.x = scale.x;
			rendererScale.y = scale.y;
		}

		for (auto& state : anims)
		{
			if (state.anim != nullptr)
				core::GarbageCollection::GetInstance()->SetRootSet(state.anim);
		}
	}
	SH_USER_API void Animator::Start()
	{
		DecideAnimation();
		animIdx = -1;
		Next();
	}
	SH_USER_API void Animator::Update()
	{
		t += world.deltaTime;

		while (!bStop && t >= nextT && nextT > 0.f) {
			t -= nextT;
			Next();
		}
	}
	SH_USER_API auto Animator::Serialize() const -> core::Json
	{
		core::Json mainJson = Super::Serialize();

		for (auto& animState : anims)
		{
			core::Json json;
			if (animState.anim == nullptr)
				json["anim"] = core::UUID::GenerateEmptyUUID().ToString();
			else
				json["anim"] = animState.anim->GetUUID().ToString();
			json["condition"] = animState.condition;

			mainJson["Animator"]["anims"].push_back(std::move(json));
		}

		return mainJson;
	}
	SH_USER_API void Animator::Deserialize(const core::Json& json)
	{
		Super::Deserialize(json);

		if (!json.contains("Animator"))
			return;
		const auto& mainJson = json["Animator"];

		if (!mainJson.contains("anims"))
			return;
		anims.resize(mainJson["anims"].size());

		int i = 0;
		for (const auto& animJson : mainJson["anims"])
		{
			if (animJson.contains("anim"))
			{
				const core::UUID animUUID{ animJson["anim"].get_ref<const std::string&>() };
				anims[i].anim = static_cast<AnimationData*>(core::SObject::GetSObjectUsingResolver(animUUID));
			}
			anims[i].condition = animJson.value("condition", 10);
			++i;
		}
	}
	SH_USER_API void Animator::SetState(int state)
	{
		if (this->state == state)
			return;
		SH_INFO_FORMAT("state: {}", state);
		this->state = state;
		DecideAnimation();
		t = 0.f;
		Next();
	}
	void Animator::DecideAnimation()
	{
		curAnim = nullptr;
		bStop = true;

		for (int i = 0; i < anims.size(); ++i)
		{
			if (anims[i].condition == state)
			{
				if (i < anims.size())
				{
					curAnim = anims[i].anim;
					bStop = false;
					animIdx = -1;
					nextT = 0.f;
					return;
				}
			}
		}
	}
	void Animator::Next()
	{
		if (curAnim == nullptr)
			return;
		++animIdx;
		if (animIdx == curAnim->GetSize())
		{
			if (!curAnim->IsLoop())
			{
				animIdx = curAnim->GetSize() - 1;
				bStop = true;
				return;
			}
			animIdx = 0;
		}
		auto texPtr = curAnim->GetTexture(animIdx);
		if (texPtr == nullptr)
			return;
		nextT = curAnim->GetDelay(animIdx) / 1000.f;

		const Vec2& offset = curAnim->GetOffset();
		const auto& pos = renderer->gameObject.transform->GetWorldPosition();
		renderer->gameObject.transform->SetPosition(rendererPos.x + offset.x, rendererPos.y + offset.y, pos.z);
		float w = texPtr->GetWidth() * 0.01f;
		float h = texPtr->GetHeight() * 0.01f;
		renderer->gameObject.transform->SetScale(w, 1.f, h);

		auto prop = renderer->GetMaterialPropertyBlock();
		prop->SetProperty("tex", texPtr);
		renderer->UpdatePropertyBlockData();
	}
}//namespace