#pragma once
#include "Export.h"

#include "Core/SContainer.hpp"

#include "Game/Component/Component.h"
#include "Game/Component/MeshRenderer.h"
#include "Game/Vector.h"

#include "Render/Texture.h"

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
			Jump
		};
	public:
		SH_USER_API PlayerAnimation(GameObject& owner);

		SH_USER_API void Awake() override;
		SH_USER_API void Update() override;

		SH_USER_API void SetPose(Pose pose);
	private:
		void ChangeTexture(float delayMs, const std::vector<render::Texture*>& texs);
	public:
		bool bRight = false;
	private:
		PROPERTY(meshRenderer)
		MeshRenderer* meshRenderer = nullptr;
		PROPERTY(idles)
		std::vector<render::Texture*> idles;
		PROPERTY(walks)
		std::vector<render::Texture*> walks;
		PROPERTY(jumps)
		std::vector<render::Texture*> jumps;
		core::SObjWeakPtr<render::Material> mat = nullptr;

		Pose curPose = Pose::Idle;

		int texIdx = 0;
		float t = 0.0f;

		Vec3 initPos;
		Vec3 posOffset;
		Vec3 initMeshScale;
	};
}//namespace