#pragma once
#if !SH_SERVER
#include "Export.h"
#include "Animation.h"

#include "Game/Component/Component.h"
#include "Game/Component/MeshRenderer.h"

namespace sh::game
{
	class MobAnimation : public Component
	{
		COMPONENT(MobAnimation, "user")
	public:
		enum class Pose
		{
			Idle = 0,
			Move = 1,
			Jump = 2,
			Attack = 3,
			Die = 4
		};
	public:
		SH_USER_API MobAnimation(GameObject& owner);
		SH_USER_API void Awake() override;
		SH_USER_API void BeginUpdate() override;

		SH_USER_API void SetPose(Pose pose);

		SH_USER_API void SetMeshRenderer(MeshRenderer& meshRenderer);
		SH_USER_API auto GetMeshRenderer() const -> MeshRenderer*;
	public:
		bool bRight = false;
	private:
		PROPERTY(meshRenderer)
		MeshRenderer* meshRenderer = nullptr;

		PROPERTY(idle)
		Animation* idle = nullptr;
		PROPERTY(move)
		Animation* move = nullptr;
		PROPERTY(jump)
		Animation* jump = nullptr;

		Pose curPose = Pose::Idle;
		core::SObjWeakPtr<Animation> curAnim = nullptr;
	};
}//namespace
#endif
