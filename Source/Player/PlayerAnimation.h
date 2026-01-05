#pragma once
#if !SH_SERVER
#include "Export.h"
#include "Player.h"
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
			Skill
		};
	public:
		SH_USER_API PlayerAnimation(GameObject& owner);
		SH_USER_API void Awake() override;
		SH_USER_API void BeginUpdate() override;

		SH_USER_API void SetPose(Pose pose);

		SH_USER_API void SetMeshRenderer(MeshRenderer& meshRenderer);
		SH_USER_API auto GetMeshRenderer() const -> MeshRenderer*;
		SH_USER_API auto GetPlayer() const -> Player*;

		/// @brief 락이 걸려 있으면 다른 상태로 바뀌지 않음
		/// @param lock 락을 걸건지
		SH_USER_API void SetLock(bool bLock);
		SH_USER_API auto IsLock() const -> bool;
	private:
		PROPERTY(player)
		Player* player = nullptr;
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

		bool bAnimLock = false;
	};
}//namespace
#endif