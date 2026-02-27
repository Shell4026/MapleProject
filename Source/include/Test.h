#pragma once
#include "Export.h"
#include "Game/Component/Component.h"
#include "Game/Component/Render/MeshRenderer.h"
#include "Core/SContainer.hpp"

namespace sh::game
{
	class Test2 : public Component
	{
		COMPONENT(Test2, "user")
	public:
		SH_USER_API Test2(GameObject& owner);

		SH_USER_API void Awake() override;
		SH_USER_API void Start() override;

		SH_USER_API void OnEnable() override;
		SH_USER_API void OnDisable() override;

		SH_USER_API void OnCollisionEnter(const Collision& collision) override;
		SH_USER_API void OnCollisionStay(const Collision& collision) override;

		SH_USER_API void OnTriggerEnter(Collider& other) override;
		SH_USER_API void OnTriggerStay(Collider& other) override;
		SH_USER_API void OnTriggerExit(Collider& other) override;

		SH_USER_API void Update() override;
	private:
		PROPERTY(num)
		int num = 10;
		PROPERTY(renderer)
		MeshRenderer* renderer = nullptr;
		PROPERTY(tex)
		render::Texture* tex = nullptr;
	};
}
