#pragma once
#include "Export.h"

#include "Core/SContainer.hpp"

#include "Game/Component/Component.h"
#include "Game/Component/MeshRenderer.h"
#include "Game/Vector.h"

#include "Render/Texture.h"
#include "Render/MaterialPropertyBlock.h"

//#undef SH_SERVER
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
		SH_USER_API void Update() override;

		SH_USER_API void SetPose(Pose pose);

		SH_USER_API void SetMeshRenderer(MeshRenderer& meshRenderer);
		SH_USER_API auto GetMeshRenderer() const -> MeshRenderer*;
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
		PROPERTY(prones)
		std::vector<render::Texture*> prones;
		PROPERTY(die)
		std::vector<render::Texture*> die;
		PROPERTY(attack0)
		std::vector<render::Texture*> attack0;
		PROPERTY(attack1)
		std::vector<render::Texture*> attack1;
		PROPERTY(attack2)
		std::vector<render::Texture*> attack2;

		PROPERTY(idleScale)
		game::Vec2 idleScale{ 1.f, 1.f };
		PROPERTY(walkScale)
		game::Vec2 walkScale{ 1.f, 1.f };
		PROPERTY(jumpScale)
		game::Vec2 jumpScale{ 1.f, 1.f };
		PROPERTY(proneScale)
		game::Vec2 proneScale{ 1.f, 1.f };
		PROPERTY(dieScale)
		game::Vec2 dieScale{ 1.f, 1.f };
		PROPERTY(attack0Scale)
		game::Vec2 attack0Scale{ 1.f, 1.f };
		PROPERTY(attack1Scale)
		game::Vec2 attack1Scale{ 1.f, 1.f };
		PROPERTY(attack2Scale)
		game::Vec2 attack2Scale{ 1.f, 1.f };

		PROPERTY(walkOffset)
		game::Vec2 walkOffset{ 0.f, 0.f };
		PROPERTY(jumpOffset)
		game::Vec2 jumpOffset{ 0.f, 0.f };
		PROPERTY(proneOffset)
		game::Vec2 proneOffset{ 0.f, 0.f };
		PROPERTY(attack0Offset)
		game::Vec2 attack0Offset{ 0.f, 0.f };
		PROPERTY(attack1Offset)
		game::Vec2 attack1Offset{ 0.f, 0.f };
		PROPERTY(attack2Offset)
		game::Vec2 attack2Offset{ 0.f, 0.f };

		PROPERTY(idleDelayMs)
		float idleDelayMs = 500;
		PROPERTY(walkDelayMs)
		float walkDelayMs = 180;
		PROPERTY(jumpDelayMs)
		float jumpDelayMs = 1000;
		PROPERTY(proneDelayMs)
		float proneDelayMs = 1000;
		PROPERTY(dieDelayMs)
		float dieDelayMs = 1000;
		PROPERTY(attack0DelayMs)
		float attack0DelayMs = 1000;
		PROPERTY(attack1DelayMs)
		float attack1DelayMs = 1000;
		PROPERTY(attack2DelayMs)
		float attack2DelayMs = 1000;

		Pose curPose = Pose::Idle;

		int texIdx = 0;
		float t = 0.0f;

		Vec3 initPos;
		Vec3 posOffset;
		Vec3 initScale;
	};
}//namespace