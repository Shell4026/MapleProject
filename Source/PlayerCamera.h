#pragma once
#include "Export.h"

#include "Game/Component/Component.h"

namespace sh::game
{
	class RigidBody;
	class Camera;
	class Transform;

	class PlayerCamera : public Component
	{
		COMPONENT(PlayerCamera, "user")
	private:
		PROPERTY(camera)
		Camera* camera = nullptr;
		PROPERTY(player)
		Transform* player = nullptr;
	public:
		SH_USER_API PlayerCamera(GameObject& owner);

		SH_USER_API void Awake() override;
		SH_USER_API void Start() override;
		SH_USER_API void FixedUpdate() override;
	};
}