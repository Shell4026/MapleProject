#include "Test.h"

#include "Game/World.h"
#include "Game/Input.h"
#include "Game/GameObject.h"
#include "Game/Component/MeshRenderer.h"

#include "Render/Mesh.h"

#include <random>

using namespace sh;
using namespace sh::game;

Test::Test(GameObject& owner) :
	Component(owner)
{
	this->canPlayInEditor = true;
}

SH_USER_API void Test::Update()
{
	if (Input::GetKeyPressed(Input::KeyCode::Space))
	{
		for (int i = 0; i < num; ++i)
		{
			auto obj = world.AddGameObject("test");
			auto renderer = obj->AddComponent<MeshRenderer>();
			auto cubeModel = static_cast<render::Model*>(core::SObjectManager::GetInstance()->GetSObject(core::UUID{"bbc4ef7ec45dce223297a224f8093f16"}));
			renderer->SetMesh(cubeModel->GetMeshes()[0]);
			
			static std::random_device seed{};
			static std::mt19937 gen{ seed() };

			std::uniform_real_distribution<float> dist(-100.0f, 100.f);
			float x = dist(seed);
			float y = dist(seed);
			float z = dist(seed);

			obj->transform->SetWorldPosition({ x, y, z });

			//objs.push_back(obj);
		}
	}
	if (Input::GetKeyPressed(Input::KeyCode::P))
	{
		GameObject* obj = world.GetGameObject("test");
		while (obj != nullptr)
		{
			obj->Destroy();
			obj = world.GetGameObject("test");
		}
	}
}
