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
	Test::Test(GameObject& owner) :
		Component(owner)
	{
	}

	SH_USER_API void Test::Awake()
	{
		SH_INFO("Awake!");
	}

	SH_USER_API void Test::Start()
	{
		SH_INFO("Start!");
	}

	SH_USER_API void Test::OnEnable()
	{
		SH_INFO_FORMAT("Enable! {}", gameObject.GetName().ToString());
	}

	SH_USER_API void Test::OnDisable()
	{
		SH_INFO_FORMAT("Disable... {}", gameObject.GetName().ToString());
	}

	SH_USER_API void Test::OnTriggerEnter(Collider& other)
	{
		SH_INFO("enter!");
	}
	SH_USER_API void Test::OnTriggerStay(Collider& other)
	{
		//SH_INFO("stay!");
	}
	SH_USER_API void Test::OnTriggerExit(Collider& other)
	{
		SH_INFO("exit!");
	}
}//namespace