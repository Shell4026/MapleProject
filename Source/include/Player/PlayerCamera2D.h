#pragma once
#if !SH_SERVER
#include "Export.h"

#include "Game/Component/Component.h"
#include "Game/Component/Render/Camera.h"

namespace sh::game
{
	class PlayerCamera2D : public Component
	{
		COMPONENT(PlayerCamera2D, "user")
	public:
		SH_USER_API PlayerCamera2D(GameObject& owner);

		SH_USER_API void Awake() override;
		SH_USER_API void Start() override;
		SH_USER_API void Update() override;

		SH_USER_API void SetPlayer(GameObject& player);
	private:
		auto SmoothDamp(float current, float target, float& currentVelocity, float smoothTime, float deltaTime) const -> float;
		void MoveToPlayer();
	private:
		PROPERTY(targetCamera)
		Camera* targetCamera = nullptr;
		PROPERTY(player)
		GameObject* player = nullptr;
		float dis = 1.0f;
		float velocityX = 0.0f;
		float velocityY = 0.0f;
		PROPERTY(smoothTime)
		float smoothTime = 1.0f;
		PROPERTY(camlimitMin)
		Vec2 camlimitMin;
		PROPERTY(camlimitMax)
		Vec2 camlimitMax;

		float lastWidth = 0.f;
		float lastHeight = 0.f;
		float cameraRawX = 0.f;
		float cameraRawY = 0.f;
		float centerX = 0.f;
		float centerY = 0.f;
	};
}
#endif