#include "RotateObject.h"

#include "Game/World.h"
#include "Game/Vector.h"

using namespace sh::game;

RotateObject::RotateObject(GameObject& owner) :
	Component(owner),
	speed(30.f)
{
}

RotateObject::~RotateObject()
{
}

SH_USER_API void RotateObject::OnEnable()
{
}

SH_USER_API void RotateObject::Update()
{
	const float dt = static_cast<float>(world.deltaTime);
	Vec3 rot = gameObject.transform->rotation;
	gameObject.transform->SetRotation(rot + Vec3{ xspeed * dt, speed * dt, zspeed * dt });
}
