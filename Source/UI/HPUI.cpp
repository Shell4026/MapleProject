#include "HPUI.h"

namespace sh::game
{
	HPUI::HPUI(GameObject& owner) :
		Component(owner)
	{
	}
	SH_USER_API void HPUI::Start()
	{
	}
	SH_USER_API void HPUI::Update()
	{
#if !SH_SERVER
		UpdateUI();
#endif
	}
	SH_USER_API void HPUI::SetMaxHp(uint32_t maxHp)
	{
		if (maxHp == 0)
			maxHp = 1;
		this->maxHp = maxHp;
	}
	SH_USER_API auto HPUI::GetMaxHp() const -> uint32_t
	{
		return maxHp;
	}
	SH_USER_API void HPUI::SetHp(uint32_t hp)
	{
		this->hp = hp;
	}
	SH_USER_API auto HPUI::GetHp() const -> uint32_t
	{
		return hp;
	}
	void HPUI::UpdateUI()
	{
		if (barPivot == nullptr)
			return;

		float percentage = static_cast<float>(hp) / static_cast<float>(maxHp);
		barPivot->SetScale(percentage, 1.0f, 1.0f);
	}
}//namespace