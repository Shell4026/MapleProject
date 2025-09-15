#pragma once
#include "Export.h"
#include "Game/Component/Component.h"

#include "Core/SContainer.hpp"

namespace sh::game
{
	class Test : public Component
	{
		COMPONENT(Test, "user")
	public:
		SH_USER_API Test(GameObject& owner);

		SH_USER_API void Awake() override;
		SH_USER_API void Start() override;

		SH_USER_API void OnEnable() override;
		SH_USER_API void OnDisable() override;

		SH_USER_API void OnTriggerEnter(Collider& other) override;
		SH_USER_API void OnTriggerStay(Collider& other) override;
		SH_USER_API void OnTriggerExit(Collider& other) override;
	private:
		PROPERTY(num)
			int num = 10;
	};
}
