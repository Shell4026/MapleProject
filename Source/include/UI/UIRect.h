#pragma once
#include "Export.h"
#include "UI/UI.h"

#include "Game/Component/Component.h"
#include "Game/Vector.h"

#include <vector>
namespace sh::game
{
	class UIRect : public UI
	{
		SCLASS(UIRect)
	public:
		SH_USER_API UIRect(GameObject& owner);

		SH_USER_API void Awake() override;
		SH_USER_API void OnDestroy() override;

		SH_USER_API auto IsContainsMouse() const -> bool;
		SH_USER_API virtual void OnHover() {};
	public:
		core::Observer<false, UIRect*> onClick;
	private:
		PROPERTY(origin)
		game::Vec2 origin{0.f, 0.f};
		PROPERTY(size)
		game::Vec2 size{1.f, 1.f};
		PROPERTY(bWorldPos)
		bool bWorldPos = false;
	};
}//namespace