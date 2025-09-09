#pragma once
#include "Export.h"
#include "Game/Component/Component.h"

#include "Core/SContainer.hpp"
class Test : public sh::game::Component
{
	COMPONENT(Test, "user")
public:
	SH_USER_API Test(sh::game::GameObject& owner);

	SH_USER_API void Awake() override;
	SH_USER_API void Start() override;
private:
	PROPERTY(num)
	int num = 10;
};