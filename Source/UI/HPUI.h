#pragma once
#include "../Export.h"

#include "Game/Component/Component.h"
#include "Game/Component/Transform.h"
namespace sh::game
{
	class HPUI : public Component
	{
		COMPONENT(HPUI, "user")
	public:
		SH_USER_API HPUI(GameObject& owner);

		SH_USER_API void Start() override;
		SH_USER_API void Update() override;

		SH_USER_API void SetMaxHp(uint32_t maxHp);
		SH_USER_API auto GetMaxHp() const -> uint32_t;
		SH_USER_API void SetHp(uint32_t hp);
		SH_USER_API auto GetHp() const -> uint32_t;
	private:
		void UpdateUI();
	private:
		PROPERTY(barPivot)
		Transform* barPivot = nullptr;

		uint32_t maxHp = 1;
		uint32_t hp = 0;
	};
}
