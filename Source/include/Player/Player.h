#pragma once
#include "Export.h"
#include "Entity.h"
#if !SH_SERVER
#include "NameTag.h"
#include "UI/InventoryUI.h"
#endif

#include "Game/Component/Component.h"
#include "Game/Component/Phys/RigidBody.h"
#include "Game/Prefab.h"

#include <cstdint>
namespace sh::game
{
	class MapleWorld;
	class Job;
	class SkillManager;
	class PlayerMovement;
	class PlayerPickup;
	class PlayerTickController;
	class Player : public Entity
	{
		COMPONENT(Player, "user")
	public:
		class MapleWorldKey
		{
			friend MapleWorld;
			MapleWorldKey() = default;
		};
	public:
		SH_USER_API Player(GameObject& owner);

		SH_USER_API void Awake() override;
		SH_USER_API void Start() override;
		SH_USER_API void Update() override;

		SH_USER_API auto GetEntityType() const -> Type override { return Type::Player; }

		SH_USER_API void SetUserUUID(const core::UUID& uuid, MapleWorldKey);
		SH_USER_API void SetCurrentWorld(MapleWorld& world) { currentWorld = &world; }

		SH_USER_API auto GetJob() const -> Job* { return job; }
		SH_USER_API auto GetSkillManager() const -> SkillManager* { return skillManager; }
		SH_USER_API auto GetMovement() const -> PlayerMovement* { return movement; }
		SH_USER_API auto GetPickup() const -> PlayerPickup* { return pickup; }
		SH_USER_API auto GetTickController() const -> PlayerTickController* { return tickController; }
		SH_USER_API auto GetTick() const -> uint64_t;
		SH_USER_API auto GetUserUUID() const -> const core::UUID& { return userUUID; }
		SH_USER_API auto GetCurrentWorld() const -> MapleWorld* { return currentWorld; }
		SH_USER_API auto IsLocal() const -> bool { return bLocal; }

#if !SH_SERVER
		SH_USER_API auto GetNameTag() const -> NameTag* { return nametag; }
#endif
	private:
		PROPERTY(job, core::PropertyOption::sobjPtr)
		Job* job = nullptr;
		PROPERTY(skillManager, core::PropertyOption::sobjPtr)
		SkillManager* skillManager = nullptr;
		PROPERTY(movement, core::PropertyOption::sobjPtr)
		PlayerMovement* movement = nullptr;
		PROPERTY(tickController, core::PropertyOption::sobjPtr)
		PlayerTickController* tickController = nullptr;
		PROPERTY(pickup, core::PropertyOption::sobjPtr)
		PlayerPickup* pickup = nullptr;
		PROPERTY(rigidbody)
		RigidBody* rigidbody = nullptr;
		/// @brief remote인 경우 empty
		core::UUID userUUID;

		MapleWorld* currentWorld = nullptr;
#if !SH_SERVER
		PROPERTY(nametag)
		NameTag* nametag = nullptr;
		PROPERTY(inventoryPrefab)
		Prefab* inventoryPrefab = nullptr;
#endif
		bool bLocal = false;
	};
}//namespace
