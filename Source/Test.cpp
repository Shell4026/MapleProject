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
}

SH_USER_API void Test::Awake()
{
	SH_INFO("Awake!");
}

SH_USER_API void Test::Start()
{
	SH_INFO("Start!");
}
