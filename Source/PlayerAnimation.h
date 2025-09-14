#pragma once
#if !SH_SERVER
#include "Export.h"

#include "Animation.h"

#include "Game/Component/Component.h"
#include "Game/Component/MeshRenderer.h"

namespace sh::game
{
	class PlayerAnimation : public Component
	{
		COMPONENT(PlayerAnimation, "user")
	public:
		enum class Pose
		{
			Idle,
			Walk,
			Jump,
			Die,
			Prone,
			Attack
		};
	public:
		SH_USER_API PlayerAnimation(GameObject& owner);
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
		PROPERTY(walk)
		Animation* walk = nullptr;
		PROPERTY(jump)
		Animation* jump = nullptr;
		PROPERTY(prone)
		Animation* prone = nullptr;

		Pose curPose = Pose::Idle;
		core::SObjWeakPtr<Animation> curAnim = nullptr;
	};
}//namespace
#endif
