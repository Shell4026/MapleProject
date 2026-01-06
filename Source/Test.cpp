#include "Test.h"

#include "Game/World.h"
#include "Game/Input.h"
#include "Game/GameObject.h"
#include "Game/Component/MeshRenderer.h"

#include "Render/Mesh.h"

#include <random>

using namespace sh;
using namespace sh::game;

namespace sh::game
{
	Test2::Test2(GameObject& owner) :
		Component(owner)
	{
		canPlayInEditor = true;
	}

	SH_USER_API void Test2::Awake()
	{
		SH_INFO("Awake!");
	}

	SH_USER_API void Test2::Start()
	{
		SH_INFO("Start!");
	}

	SH_USER_API void Test2::OnEnable()
	{
		SH_INFO_FORMAT("Enable! {}", gameObject.GetName().ToString());
	}

	SH_USER_API void Test2::OnDisable()
	{
		SH_INFO_FORMAT("Disable... {}", gameObject.GetName().ToString());
	}

	SH_USER_API void Test2::OnTriggerEnter(Collider& other)
	{
		SH_INFO("enter!");
	}
	SH_USER_API void Test2::OnTriggerStay(Collider& other)
	{
		//SH_INFO("stay!");
	}
	SH_USER_API void Test2::OnTriggerExit(Collider& other)
	{
		SH_INFO("exit!");
	}
	SH_USER_API void Test2::Update()
	{
		Super::Update();

		if (Input::GetKeyPressed(Input::KeyCode::K))
		{
			if (core::IsValid(renderer))
			{
				auto prop = renderer->GetMaterialPropertyBlock();
				if (prop == nullptr)
				{
					renderer->SetMaterialPropertyBlock(std::make_unique<render::MaterialPropertyBlock>());
					prop = renderer->GetMaterialPropertyBlock();
				}
				prop->SetProperty("tex", tex);
				renderer->UpdatePropertyBlockData();
			}
		}
		if (Input::GetKeyPressed(Input::KeyCode::Enter))
		{
			for (int i = 0; i < 10000; ++i)
			{
				auto obj = world.AddGameObject("test" + std::to_string(i));
				obj->transform->SetParent(gameObject.transform);
			}
		}
	}
}//namespace