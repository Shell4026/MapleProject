#include "Skill.h"
#include "PlayerSkillManager.h"
#include "Packet/SkillUsingPacket.h"
#include "Packet/SkillStatePacket.h"
#include "MapleClient.h"
#include "MapleServer.h"

#include "Game/GameObject.h"

namespace sh::game
{
	Skill::Skill(GameObject& owner) :
		NetworkComponent(owner)
	{
		rnd.seed(std::random_device{}());
	}
	SH_USER_API void Skill::Awake()
	{
		if (skillManager != nullptr)
			skillManager->Register(*this);
#if !SH_SERVER
		if (animator != nullptr)
		{
			for (auto anim : anims)
			{
				if (anim == nullptr)
					continue;
				anim->SetTarget(*animator->gameObject.transform);
			}
		}
#endif
	}
	SH_USER_API void Skill::BeginUpdate()
	{
#if !SH_SERVER
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
#endif
	}
	SH_USER_API void Skill::Update()
	{
		if (!core::IsValid(playerMovement))
			return;

		//static MapleServer* server = MapleServer::GetInstance();
		if (bUsing)
		{
			delay -= static_cast<int>(world.deltaTime * 1000.f);
			if (delay <= 0)
			{
				bUsing = false;
				if (!bCanMove)
					playerMovement->Unlock();
#if !SH_SERVER
				if (animator != nullptr)
					animator->SetLock(false);
#endif
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
	SH_USER_API void Skill::Use()
	{
#if !SH_SERVER
		static MapleClient* client = MapleClient::GetInstance();
		if (bCanUse && !bUsing)
		{
			SkillUsingPacket packet{};
			packet.userUUID = client->GetUser().GetUserUUID().ToString();
			packet.skillId = id;
			client->SendPacket(packet);
			PlayAnim();
			animator->SetLock(true);
		}
#endif
		if (bCanUse && !bUsing)
		{
			SH_INFO("Use!");
			if (core::IsValid(playerMovement) && !bCanMove)
				playerMovement->Lock();
			delay = delayMs;
			cooldown = cooldownMs;
			bCanUse = false;
			bUsing = true;

#if SH_SERVER
			SkillStatePacket packet{};
			packet.userUUID = skillManager->GetPlayer()->GetUserUUID().ToString();
			packet.skillId = id;
			packet.bUsing = true;

			static MapleServer* server = MapleServer::GetInstance();
			server->BroadCast(packet);
#endif
		}
	}
#if !SH_SERVER
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

		SH_INFO_FORMAT("me: {}, packet: {}", player->GetUserUUID().ToString(), packet.userUUID);

		if (player->IsLocal())
			return;
		if (player->GetUserUUID().ToString() == packet.userUUID)
		{
			SH_INFO("remote");
			if (!bUsing)
				PlayAnim();
		}
	}
#endif
#if !SH_SERVER
	void Skill::PlayAnim()
	{
		if (anims.empty() || !core::IsValid(animator) || !core::IsValid(animator->GetMeshRenderer()))
			return;

		animator->SetPose(PlayerAnimation::Pose::Skill);

		int motionIdx = std::uniform_int_distribution<int>{ 0, static_cast<int>(anims.size()) - 1 }(rnd);
		if (curAnim.IsValid())
			curAnim->Stop();

		curAnim = anims[motionIdx];
		curAnim->InverseX(animator->bRight);
		curAnim->Play(*animator->GetMeshRenderer());
	}
#endif
}//namespace