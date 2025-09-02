#include "PlayerCamera.h"
#include "Game/Component/Camera.h"

#include "Game/GameObject.h"
#include "Game/Input.h"

#include "Core/Logger.h"

namespace sh::game
{
	PlayerCamera::PlayerCamera(GameObject& owner) :
		Component(owner)
	{
	}
	SH_USER_API void PlayerCamera::Awake()
	{
		Super::Awake();
		if (camera == nullptr)
			camera = gameObject.GetComponent<Camera>();

		SH_INFO("Awake?");
	}
	SH_USER_API void PlayerCamera::Start()
	{
		SH_INFO("Start?");
	}
	SH_USER_API void PlayerCamera::FixedUpdate()
	{
		if (!core::IsValid(camera) || !core::IsValid(player))
			return;

		auto& playerPos = player->GetWorldPosition();
		camera->SetLookPos(playerPos);

		Vec3 camPos = playerPos;
		camPos.y += 2;
		camPos.z += 2;
		camera->gameObject.transform->SetPosition(camPos);
	}
}//namespace