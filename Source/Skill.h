#pragma once
#include "Export.h"
#include "Animation.h"
#include "PlayerMovement2D.h"
#include "PlayerSkillManager.h"

#include "Core/SContainer.hpp"

#include "Game/Component/NetworkComponent.h"
#include "Game/Component/MeshRenderer.h"
#include "Game/Input.h"

#include <random>
#include <vector>
//#define SH_SERVER 1
namespace sh::game
{
	class SkillStatePacket;
	class SkillHitbox;

	class Skill : public NetworkComponent
	{
		COMPONENT(Skill, "user")
	public:
		SH_USER_API Skill(GameObject& owner);
		
		SH_USER_API void Awake() override;
		SH_USER_API void Start() override;
		SH_USER_API void BeginUpdate() override;
		SH_USER_API void Update() override;

		SH_USER_API auto GetId() const -> uint32_t;
		SH_USER_API auto IsUsing() const -> bool;

		SH_USER_API virtual void Use();

#if !SH_SERVER
		SH_USER_API void SetKey(Input::KeyCode keyCode);
		SH_USER_API void ProcessState(const SkillStatePacket& packet);
#endif
	private:
#if !SH_SERVER
		void PlayAnim();
#endif
	protected:
#if !SH_SERVER
		PROPERTY(animator)
		PlayerAnimation* animator = nullptr;
		PROPERTY(anims)
		std::vector<Animation*> anims;
		
		core::SObjWeakPtr<Animation> curAnim = nullptr;
#endif
		PROPERTY(playerMovement)
		PlayerMovement2D* playerMovement = nullptr;
	private:
		std::mt19937 rnd;

		PROPERTY(skillManager)
		PlayerSkillManager* skillManager = nullptr;
		PROPERTY(id)
		uint32_t id = 0;
		PROPERTY(delayMs)
		uint32_t delayMs = 800;
		PROPERTY(cooldownMs)
		uint32_t cooldownMs = 0;
		PROPERTY(damage)
		float damage = 10.f;
		PROPERTY(hitboxes)
		std::vector<SkillHitbox*> hitboxes;

		int delay = 0;
		int cooldown = 0;
#if !SH_SERVER
		PROPERTY(keys)
		std::vector<char> keys;
#endif
		bool bCanUse = true;
		bool bUsing = false;
		PROPERTY(bCanMove)
		bool bCanMove = false;
	};
}//namespace