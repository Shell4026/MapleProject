#include "Skill/Skill.h"
#include "Packet/SkillUsingPacket.hpp"
#include "Packet/SkillStatePacket.hpp"
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

				for (auto hitbox : hitboxes)
				{
					if (core::IsValid(hitbox))
					{
						hitbox->gameObject.SetActive(false);
						hitbox->gameObject.transform->UpdateMatrix();
					}
				}
			}
			if (hitboxt <= 0 && bUsing)
			{
				for (auto hitbox : hitboxes)
				{
					if (core::IsValid(hitbox))
					{
						auto hitboxPos = hitbox->gameObject.transform->position;
						if (playerMovement->GetPlayer()->IsRight() && hitboxPos.x < 0 ||
							!playerMovement->GetPlayer()->IsRight() && hitboxPos.x > 0)
						{
							hitboxPos.x *= -1.0f;
							hitbox->gameObject.transform->SetPosition(hitboxPos);
						}
						hitbox->gameObject.SetActive(true);
						hitbox->gameObject.transform->UpdateMatrix();
					}
				}
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
		if (bCanUse && !bUsing)
		{
			if (core::IsValid(playerMovement) && !bCanMove)
				playerMovement->Lock();
			delay = delayMs;
			cooldown = cooldownMs;
			bCanUse = false;
			bUsing = true;
			hitboxt = hitBoxMs;

			SkillStatePacket packet{};
			packet.userUUID = skillManager->GetPlayer()->GetUserUUID();
			packet.skillId = id;
			packet.bUsing = true;

			static MapleServer* server = MapleServer::GetInstance();
			server->BroadCast(packet);
		}
	}
	SH_USER_API auto Skill::GetDamage() const -> float
	{
		return damage;
	}
}//namespace