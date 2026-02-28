#pragma once
#include "Export.h"
#include "AnimationData.h"

#include "Core/GCObject.h"

#include "Game/Component/Component.h"
#include "Game/Component/Render/MeshRenderer.h"
#include "Game/Vector.h"

namespace sh::editor
{
	class AnimatorInspector;
}
namespace sh::game
{
	class Animator : public Component
	{
		COMPONENT(Animator, "user")
		friend editor::AnimatorInspector;
	public:
		SH_USER_API Animator(GameObject& owner);
		
		SH_USER_API void Awake() override;
		SH_USER_API void Start() override;
		SH_USER_API void Update() override;
		SH_USER_API auto Serialize() const -> core::Json override;
		SH_USER_API void Deserialize(const core::Json& json) override;

		SH_USER_API void SetState(int state);
		SH_USER_API auto GetState() const -> int { return state; }
		SH_USER_API auto GetMeshRenderer() const -> MeshRenderer* { return renderer; }
		SH_USER_API auto GetCurAnimation() const -> AnimationData* { return curAnim; }
	private:
		void DecideAnimation();
		void Next();
	protected:
		PROPERTY(renderer)
		MeshRenderer* renderer = nullptr;
	private:
		struct AnimState : core::GCObject
		{
			AnimationData* anim = nullptr;
			int condition = 0;

			SH_USER_API void PushReferenceObjects(core::GarbageCollection& gc) override;
		};
		std::vector<AnimState> anims;

		PROPERTY(curAnim, core::PropertyOption::invisible)
		AnimationData* curAnim = nullptr;

		int state = 0;

		int animIdx = 0;
		float nextT = 0;
		float t = 0.f;

		bool bStop = false;
	};
}//namespace