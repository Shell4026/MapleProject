#pragma once
#if !SH_SERVER
#include "Export.h"

#include "Core/SContainer.hpp"

#include "Game/Component/Transform.h"
#include "Game/Component/Component.h"
#include "Game/Component/MeshRenderer.h"
#include "Game/Vector.h"

#include "Render/Texture.h"

#include <vector>
namespace sh::game
{
	class Animation : public Component
	{
		COMPONENT(Animation, "user")
	public:
		SH_USER_API Animation(GameObject& owner);
		SH_USER_API void SetTarget(Transform& transform);

		SH_USER_API void InverseX(bool bInverse);
		SH_USER_API void Play(MeshRenderer& meshRenderer);
		SH_USER_API void Stop();

		SH_USER_API void SetLoop(bool bLoop);
		SH_USER_API auto IsLoop() const -> bool;

		SH_USER_API auto IsPlaying() const -> bool;
		SH_USER_API auto GetCurrentTexture() const -> render::Texture*;

		SH_USER_API void Update() override;
	private:
		PROPERTY(textures)
		std::vector<render::Texture*> textures;
		PROPERTY(scale)
		game::Vec2 scale{ 1.f, 1.f };
		PROPERTY(offset)
		game::Vec2 offset{ 0.f, 0.f };
		PROPERTY(delays)
		std::vector<uint32_t> delays;
		core::SObjWeakPtr<Transform> target = nullptr;
		core::SObjWeakPtr<MeshRenderer> meshRenderer = nullptr;

		game::Vec3 initPos;
		game::Vec3 initScale;

		uint32_t idx = 0;
		float t = 0;

		PROPERTY(bLoop)
		bool bLoop = false;
		bool bPlaying = false;
		bool bRight = false;
	};
}//namespace
#endif