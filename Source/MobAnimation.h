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
			Die = 4,
			Hit = 5
		};
	public:
		SH_USER_API MobAnimation(GameObject& owner);
		SH_USER_API void Awake() override;
		SH_USER_API void BeginUpdate() override;

		SH_USER_API void SetPose(Pose pose);
		SH_USER_API auto GetPose() const -> Pose;

		SH_USER_API void SetMeshRenderer(MeshRenderer& meshRenderer);
		SH_USER_API auto GetMeshRenderer() const -> MeshRenderer*;

		/// @brief 락이 걸려 있으면 다른 상태로 바뀌지 않음
		/// @param lock 락을 걸건지
		SH_USER_API void SetLock(bool bLock);
		SH_USER_API auto IsLock() const -> bool;
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
		PROPERTY(hit)
		Animation* hit = nullptr;

		Pose curPose = Pose::Idle;
		core::SObjWeakPtr<Animation> curAnim = nullptr;

		bool bAnimLock = false;
	};
}//namespace
#endif
