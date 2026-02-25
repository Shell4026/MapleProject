#pragma once
#include "Export.h"
#if !SH_SERVER
#include "NameTag.h"
#include "UI/InventoryUI.h"
#endif

#include "Game/Component/Component.h"
#include "Game/Prefab.h"
namespace sh::game
{
	class MapleWorld;
	class Player : public Component
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

		SH_USER_API void Start() override;

		SH_USER_API void SetUserUUID(const core::UUID& uuid);
		SH_USER_API auto GetUserUUID() const -> const core::UUID&;

		SH_USER_API auto IsLocal() const -> bool;

		SH_USER_API void SetRight(bool bRight);
		SH_USER_API auto IsRight() const -> bool;

		SH_USER_API void SetUserUUID(const core::UUID& uuid, MapleWorldKey);
		SH_USER_API void SetCurrentWorld(MapleWorld& world) { currentWorld = &world; }

		SH_USER_API auto GetSkillManager() const -> SkillManager* { return skillManager; }
		SH_USER_API auto GetMovement() const -> PlayerMovement* { return movement; }
		SH_USER_API auto GetUserUUID() const -> const core::UUID& { return userUUID; }
		SH_USER_API auto GetCurrentWorld() const -> MapleWorld* { return currentWorld; }
		SH_USER_API auto IsLocal() const -> bool { return bLocal; }

#if !SH_SERVER
		SH_USER_API auto GetNameTag() const -> NameTag* { return nametag; }
#endif
	private:
		PROPERTY(skillManager, core::PropertyOption::sobjPtr)
		SkillManager* skillManager = nullptr;
		PROPERTY(movement, core::PropertyOption::sobjPtr)
		PlayerMovement* movement = nullptr;

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