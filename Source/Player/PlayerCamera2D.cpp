#include "PlayerCamera2D.h"
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
	SH_USER_API void PlayerCamera2D::BeginUpdate()
	{
		Super::BeginUpdate();

		if (!core::IsValid(targetCamera))
			return;

		SetCameraZ();
	}
	SH_USER_API void PlayerCamera2D::Update()
	{
		if (!core::IsValid(player))
			return;

		MoveY();
		MoveToPlayer();
	}
	SH_USER_API void PlayerCamera2D::SetPlayer(GameObject& player)
	{
		this->player = &player;
	}
	void PlayerCamera2D::SetCameraZ()
	{
		const Vec3& camPos = targetCamera->gameObject.transform->GetWorldPosition();
		Vec3 lookPos = targetCamera->GetLookPos();
		lookPos.x = camPos.x;
		lookPos.y = camPos.y;
		lookPos.z = 0;
		targetCamera->SetLookPos(lookPos);

		const float w = gameObject.world.renderer.GetWidth();
		const float h = gameObject.world.renderer.GetHeight();
		const float worldWidth = (camlimitMax.x - camlimitMin.x) * 100;
		const float worldHeight = (camlimitMax.y - camlimitMin.y) * 100;

		dis = std::min(h, worldHeight) * 0.01f;
		Vec3 pos = camPos;
		pos.z = lookPos.z + dis;

		targetCamera->gameObject.transform->SetWorldPosition(pos);
	}
	void PlayerCamera2D::MoveY()
	{
		const float py = player->transform->GetWorldPosition().y;

		constexpr float upOffset = 0.81f; // 플레이어가 centerY보다 upOffset 위로 가면 카메라를 올림 (playerY + upOffset)
		constexpr float downOffset = upOffset * 2.0f; // centerY가 playerY보다 이만큼 더 높으면 카메라를 내림 (playerY + downOffset)

		float desiredCenterY = centerY;

		if (py > centerY + upOffset)
			desiredCenterY = py + upOffset;
		else if (centerY > py + downOffset)
			desiredCenterY = py + downOffset;

		centerY = desiredCenterY;
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
		float width = world.renderer.GetWidth() / 100.f;
		float height = dis;

		auto pos = gameObject.transform->GetWorldPosition();
		centerX = pos.x;

		const float worldWidth = camlimitMax.x - camlimitMin.x;
		const float worldHeight = camlimitMax.y - camlimitMin.y;

		float minX = camlimitMin.x;
		float maxX = camlimitMax.x;
		float minY = camlimitMin.y;
		float maxY = camlimitMax.y;

		if (worldWidth <= width)
			minX = maxX = (minX + maxX) * 0.5f;
		else 
		{
			minX = minX + width * 0.5f;
			maxX = maxX - width * 0.5f;
		}

		if (worldHeight <= height)
			minY = maxY = (minY + maxY) * 0.5f;
		else 
		{
			minY = minY + height * 0.5f;
			maxY = maxY - height * 0.5f;
		}

		if (pos.x >= minX && pos.x <= maxX)
			pos.x = SmoothDamp(pos.x, centerX, velocityX, smoothTime, world.deltaTime);

		if (pos.y >= minY && pos.y <= maxY)
			pos.y = SmoothDamp(pos.y, centerY, velocityY, smoothTime, world.deltaTime);

		pos.x = std::clamp(pos.x, minX, maxX);
		pos.y = std::clamp(pos.y, minY, maxY);

		gameObject.transform->SetWorldPosition(pos);
	}
}//namespace
#endif