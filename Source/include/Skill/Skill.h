#pragma once
#include "Export.h"
#include "Player/PlayerMovement2D.h"
#include "SkillHitbox.h"

#if !SH_SERVER
#include "Player/PlayerAnimation.h"
#include "Animation.h"
#endif

#include "Core/SContainer.hpp"

#include "Game/Component/Component.h"
#include "Game/Component/Render/MeshRenderer.h"
#include "Game/Input.h"

#include <random>
#include <vector>
namespace sh::game
{
	class SkillStatePacket;

	class Skill : public Component
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

		SH_USER_API void Deserialize(const core::Json& json) override;

		SH_USER_API virtual void Use();

		SH_USER_API auto GetDamage() const -> float;
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
		PROPERTY(id)
		uint32_t id = 0;
		PROPERTY(delayMs)
		uint32_t delayMs = 800;
		PROPERTY(cooldownMs)
		uint32_t cooldownMs = 0;
		PROPERTY(damage)
		float damage = 10.f;
		PROPERTY(hitBoxMs)
		uint32_t hitBoxMs = 350;
		PROPERTY(hitboxes)
		std::vector<SkillHitbox*> hitboxes;

		core::EventSubscriber<network::PacketEvent> packetSubscriber;

		int hitboxt = 0;
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