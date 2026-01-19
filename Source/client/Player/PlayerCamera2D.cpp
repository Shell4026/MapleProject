#include "Player/PlayerCamera2D.h"
#if !SH_SERVER
#include "Game/GameObject.h"

#include "Window/Window.h"
namespace sh::game
{
	PlayerCamera2D::PlayerCamera2D(GameObject& owner) :
		Component(owner)
	{
	}
	SH_USER_API void PlayerCamera2D::Start()
	{
		world.renderer.GetWindow().SetSize(1366, 768);

		if (core::IsValid(player))
		{
			const float px = player->transform->GetWorldPosition().x;
			const float py = player->transform->GetWorldPosition().y;
			centerX = px;
			centerY = py + 0.81f;
		}
	}
	SH_USER_API void PlayerCamera2D::LateUpdate()
	{
		Super::BeginUpdate();

		if (!core::IsValid(targetCamera))
			return;
		if (!core::IsValid(player))
			return;

		MoveToPlayer();
		targetCamera->GetNative().UpdateMatrix();
	}
	SH_USER_API void PlayerCamera2D::SetPlayer(GameObject& player)
	{
		this->player = &player;
	}
	auto PlayerCamera2D::SmoothDamp(float current, float target, float& currentVelocity, float smoothTime, float deltaTime) const -> float
	{
		float omega = 2.0f / smoothTime;
		float x = omega * deltaTime;
		float exp = 1.0f / (1.0f + x + 0.48f * x * x + 0.235f * x * x * x);

		float change = current - target;
		float temp = (currentVelocity + omega * change) * deltaTime;
		currentVelocity = (currentVelocity - omega * temp) * exp;

		float result = target + (change + temp) * exp;
		return result;
	}
	void PlayerCamera2D::MoveToPlayer()
	{
		const float width = world.renderer.GetWidth() * 0.01f;
		const float height = world.renderer.GetHeight() * 0.01f;

		const float px = player->transform->GetWorldPosition().x;
		const float py = player->transform->GetWorldPosition().y;
		const float upOffset = 0.81f; // 플레이어가 centerY보다 upOffset 위로 가면 카메라를 올림 (playerY + upOffset)
		const float downOffset = upOffset * 2.0f; // centerY가 playerY보다 이만큼 더 높으면 카메라를 내림 (playerY + downOffset)

		centerX = px;
		if (py > centerY + upOffset)
			centerY = py + upOffset;
		else if (centerY > py + downOffset)
			centerY = py + downOffset;

		const float minX = camlimitMin.x + width * 0.5f;
		const float maxX = camlimitMax.x - width * 0.5f;
		const float minY = camlimitMin.y + height * 0.5f;
		const float maxY = camlimitMax.y - height * 0.5f;

		if (minX < maxX)
			centerX = std::clamp(centerX, minX, maxX);
		else
			centerX = (camlimitMin.x + camlimitMax.x) * 0.5f;

		if (minY < maxY)
			centerY = std::clamp(centerY, minY, maxY);
		else
			centerY = (camlimitMin.y + camlimitMax.y) * 0.5f;

		auto pos = targetCamera->gameObject.transform->GetWorldPosition();
		pos.x = SmoothDamp(pos.x, centerX, velocityX, smoothTime, world.deltaTime);
		pos.y = SmoothDamp(pos.y, centerY, velocityY, smoothTime, world.deltaTime);
		const float unitsPerPixel = 0.001f;
		pos.x = std::round(pos.x / unitsPerPixel) * unitsPerPixel;
		pos.y = std::round(pos.y / unitsPerPixel) * unitsPerPixel;
		pos.z = 0;

		targetCamera->SetLookPos(pos);

		const float w = gameObject.world.renderer.GetWidth();
		const float h = gameObject.world.renderer.GetHeight();

		dis = h * 0.01f;
		pos.z = dis;

		targetCamera->gameObject.transform->SetWorldPosition(pos);
		targetCamera->gameObject.transform->UpdateMatrix();
	}
}//namespace
#endif