#include "Player/Player.h"
#include "Player/Job.h"
#include "Player/PlayerMovement.h"
#include "Skill/SkillManager.h"

#include "Game/GameObject.h"
#if SH_SERVER
#else
#include "MapleClient.h"
#endif
namespace sh::game
{
	Player::Player(GameObject& owner) :
		Entity(owner), userUUID(core::UUID::GenerateEmptyUUID())
	{
	}
	SH_USER_API void Player::Awake()
	{
		if (job == nullptr)
			SH_ERROR("job is nullptr!");
		if (skillManager == nullptr)
			SH_ERROR("skillManager is nullptr!");
		if (rigidbody == nullptr)
			SH_ERROR("rigidbody is nullptr!");

		tickController.RegisterTickable(movement);
		tickController.RegisterTickable(skillManager);

		for (Skill* skill : job->GetSkills())
		{
			if (skill == nullptr)
				continue;
			skillManager->RegisterSkill(*skill);
		}
	}
	SH_USER_API void Player::Start()
	{
#if !SH_SERVER
		if (bLocal)
		{
			if (inventoryPrefab == nullptr)
				SH_ERROR("Inventory prefab is nullptr");
			else
				inventoryPrefab->AddToWorld(world);
		}
#endif
	}
	SH_USER_API void Player::BeginUpdate()
	{
		tickController.BeginUpdate();
	}
	SH_USER_API void Player::FixedUpdate()
	{
		tickController.FixedUpdate();
	}
	SH_USER_API void Player::Update()
	{
		rigidbody->ResetPhysicsTransform();
		tickController.Update();
	}
	SH_USER_API auto Player::GetTick() const -> uint64_t
	{
		return tickController.GetTick();
	}
	SH_USER_API void Player::SetUserUUID(const core::UUID& uuid, MapleWorldKey)
	{
		userUUID = uuid;
#if !SH_SERVER
		bLocal = MapleClient::GetUser().GetUserUUID() == uuid;
#endif
	}
}//namespace
