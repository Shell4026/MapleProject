#include "System/Animator.h"

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
			auto propBlock = renderer->GetMaterialPropertyBlock();
			if (propBlock->GetVectorProperty("color") != nullptr)
			{
				propBlock->SetProperty("color", glm::vec4{ 1.f, 1.f, 1.f, 1.f });
				renderer->UpdatePropertyBlockData();
			}
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

		ChangeAlpha();
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
		this->state = state;
		DecideAnimation();
		t = 0.f;
		Next();
	}
	SH_USER_API auto Animator::GetCurAnimationPos() const -> const Vec2&
	{
		static Vec2 zero{ 0.f, 0.f };
		if (curAnim == nullptr)
			return zero;
		return curAnim->GetPos(animIdx);
	}
	void Animator::DecideAnimation()
	{
		curAnim = nullptr;
		bStop = true;

		for (int i = 0; i < anims.size(); ++i)
		{
			if (anims[i].condition == state)
			{
				curAnim = anims[i].anim;
				bStop = false;
				animIdx = -1;
				nextT = 0.f;
				return;
			}
		}
	}
	void Animator::Next()
	{
		if (curAnim == nullptr)
			return;
		if (curAnim->GetSize() == 0)
		{
			bStop = true;
			animIdx = 0;
			nextT = 0.f;
			return;
		}
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

		const Vec2& animPos = curAnim->GetPos(animIdx);
		renderer->gameObject.transform->SetPosition(animPos.x, animPos.y, renderer->gameObject.transform->position.z);
		float w = texPtr->GetWidth() * 0.01f;
		float h = texPtr->GetHeight() * 0.01f;
		renderer->gameObject.transform->SetScale(w, 1.f, h);
		renderer->gameObject.transform->UpdateMatrix();

		auto prop = renderer->GetMaterialPropertyBlock();
		prop->SetProperty("tex", texPtr);
		renderer->UpdatePropertyBlockData();
	}

	void Animator::ChangeAlpha()
	{
		if (curAnim == nullptr)
			return;

		const uint8_t a0 = curAnim->GetAlphaSrc(animIdx);
		const uint8_t a1 = curAnim->GetAlphaDst(animIdx);

		auto prop = renderer->GetMaterialPropertyBlock();
		auto colorPtr = prop->GetVectorProperty("color");
		glm::vec4 color = { 1.f, 1.f, 1.f, 1.f };
		if (colorPtr != nullptr)
			color = *colorPtr;

		if (nextT > 0.f)
		{
			const float alpha = t / nextT;
			color.a = glm::mix(a0, a1, alpha) / 255.f;
		}
		else
			color.a = a0 / 255.f;

		prop->SetProperty("color", color);
		renderer->UpdatePropertyBlockData();
	}

	SH_USER_API void Animator::AnimState::PushReferenceObjects(core::GarbageCollection& gc)
	{
		gc.PushReferenceObject(anim);
	}
}//namespace
