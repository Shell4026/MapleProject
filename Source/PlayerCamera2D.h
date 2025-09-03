#pragma once
#include "Export.h"

#include "Game/Component/Component.h"
#include "Game/Component/Camera.h"

namespace sh::game
{
	class PlayerCamera2D : public Component
	{
		COMPONENT(PlayerCamera2D, "user")
	public:
		SH_USER_API PlayerCamera2D(GameObject& owner);

		SH_USER_API void Start() override;
		SH_USER_API void BeginUpdate() override;
		SH_USER_API void Update() override;
		SH_USER_API void LateUpdate() override;
	private:
		auto SmoothDamp(float current, float target, float& currentVelocity, float smoothTime, float deltaTime) const -> float;
		void MoveToPlayer();
	private:
		PROPERTY(targetCamera)
		Camera* targetCamera = nullptr;
		PROPERTY(player)
		GameObject* player = nullptr;
		float velocityX = 1.0f;
		float velocityY = 1.0f;
		PROPERTY(smoothTime)
		float smoothTime = 1.0f;
		PROPERTY(minX)
		float minX = -100;
		PROPERTY(maxX)
		float maxX = 100;
		PROPERTY(minY)
		float minY = -100;
		PROPERTY(maxY)
		float maxY = 100;

		float centerX = 0.f;
		float centerY = 0.f;
	};
}