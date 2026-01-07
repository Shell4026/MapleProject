#include "Skill/Skill.h"
#include "Packet/SkillUsingPacket.hpp"
#include "Packet/SkillStatePacket.hpp"
#include "MapleClient.h"
#include "MapleServer.h"
#include "Skill/SkillHitbox.h"

#include "Core/Util.h"

#include "Game/GameObject.h"

namespace sh::game
{
	Skill::Skill(GameObject& owner) :
		NetworkComponent(owner)
	{
	}
	SH_USER_API void Skill::Awake()
	{
		if (skillManager != nullptr)
			skillManager->Register(*this);
		if (animator != nullptr)
		{
			for (auto anim : anims)
			{
				if (anim == nullptr)
					continue;
				anim->SetTarget(*animator->gameObject.transform);
			}
		}
	}
	SH_USER_API void Skill::Start()
	{
		for (auto hitbox : hitboxes)
		{
			if (core::IsValid(hitbox))
			{
				hitbox->gameObject.SetActive(false);
				hitbox->gameObject.transform->UpdateMatrix();
			}
		}
	}
	SH_USER_API void Skill::BeginUpdate()
	{
		if (skillManager == nullptr || !skillManager->GetPlayer()->IsLocal())
			return;
		if (!keys.empty())
		{
			for (auto keyChar : keys)
			{
				Input::KeyCode keyCode = static_cast<Input::KeyCode>((keyChar - 'A') + static_cast<int>(Input::KeyCode::A));
				if (Input::GetKeyDown(keyCode))
					Use();
			}
		}
	}
	SH_USER_API void Skill::Update()
	{
		if (!core::IsValid(playerMovement))
			return;

		//static MapleServer* server = MapleServer::GetInstance();
		if (bUsing)
		{
			delay -= static_cast<int>(world.deltaTime * 1000.f);
			hitboxt -= static_cast<int>(world.deltaTime * 1000.f);
			if (delay <= 0)
			{
				bUsing = false;
				if (!bCanMove)
					playerMovement->Unlock();
				if (animator != nullptr)
					animator->SetLock(false);
			}
		}
		if (!bCanUse)
		{
			cooldown -= static_cast<int>(world.deltaTime * 1000.f);
			if (cooldown <= 0)
				bCanUse = true;
		}
	}
	SH_USER_API auto Skill::GetId() const -> uint32_t
	{
		return id;
	}
	SH_USER_API auto Skill::IsUsing() const -> bool
	{
		return bUsing;
	}
	SH_USER_API void Skill::Deserialize(const core::Json& json)
	{
		Super::Deserialize(json);
	}
	SH_USER_API void Skill::Use()
	{
		SH_INFO("use");
		static MapleClient* client = MapleClient::GetInstance();
		if (bCanUse && !bUsing)
		{
			SkillUsingPacket packet{};
			packet.userUUID = client->GetUser().GetUserUUID();
			packet.skillId = id;
			client->SendPacket(packet);

			PlayAnim();
			animator->SetLock(true);
			if (core::IsValid(playerMovement) && !bCanMove)
				playerMovement->Lock();

			delay = delayMs;
			cooldown = cooldownMs;
			bCanUse = false;
			bUsing = true;
		}
	}
	SH_USER_API auto Skill::GetDamage() const -> float
	{
		return damage;
	}
	SH_USER_API void Skill::SetKey(Input::KeyCode keyCode)
	{
		keys.clear();
	}
	SH_USER_API void Skill::ProcessState(const SkillStatePacket& packet)
	{
		if (skillManager == nullptr)
			return;
		Player* player = skillManager->GetPlayer();
		if (!core::IsValid(player))
			return;

		if (player->IsLocal())
			return;
		if (player->GetUserUUID() == packet.userUUID)
		{
			if (!bUsing)
				PlayAnim();
		}
	}
	void Skill::PlayAnim()
	{
		if (anims.empty() || !core::IsValid(animator) || !core::IsValid(animator->GetMeshRenderer()) || !core::IsValid(animator->GetPlayer()))
			return;

		animator->SetPose(PlayerAnimation::Pose::Skill);

		int motionIdx = core::Util::RandomRange(0, static_cast<int>(anims.size()) - 1);
		if (curAnim.IsValid())
			curAnim->Stop();

		curAnim = anims[motionIdx];
		curAnim->InverseX(animator->GetPlayer()->IsRight());
		curAnim->Play(*animator->GetMeshRenderer());
	}
}//namespace