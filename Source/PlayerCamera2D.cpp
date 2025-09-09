#include "PlayerCamera2D.h"

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
#if !SH_SERVER
		world.renderer.GetWindow().SetSize(1920, 1080);

		if (core::IsValid(player))
		{
			const float px = player->transform->GetWorldPosition().x;
			const float py = player->transform->GetWorldPosition().y;
			centerX = px;
			centerY = py + 0.81f;
		}
#endif
	}
	SH_USER_API void PlayerCamera2D::BeginUpdate()
	{
#if !SH_SERVER
		Super::BeginUpdate();

		if (!core::IsValid(targetCamera))
			return;

		const Vec3& camPos = targetCamera->gameObject.transform->GetWorldPosition();
		Vec3 lookPos = targetCamera->GetLookPos();
		lookPos.x = camPos.x;
		lookPos.y = camPos.y;
		lookPos.z = 0;
		targetCamera->SetLookPos(lookPos);

		const float w = gameObject.world.renderer.GetWidth();
		const float h = gameObject.world.renderer.GetHeight();

		dis = h * 0.01f;
		Vec3 pos = camPos;
		pos.z = lookPos.z + dis;

		targetCamera->gameObject.transform->SetWorldPosition(pos);
#endif
	}
	SH_USER_API void PlayerCamera2D::Update()
	{
#if !SH_SERVER
		if (!core::IsValid(player))
			return;
		const float px = player->transform->GetWorldPosition().x;
		const float py = player->transform->GetWorldPosition().y;
		if (py + 0.81f > centerY)
			centerY = SmoothDamp(centerY, py + 0.81f, velocityY, smoothTime, world.deltaTime);
		if (centerY > py + 0.81f * 2.0f)
			centerY = SmoothDamp(centerY, py + 0.81f, velocityY, smoothTime, world.deltaTime);

		centerX = px;

		MoveToPlayer();
#endif
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
#if !SH_SERVER
		float width = world.renderer.GetWidth() / 100.f;
		float height = dis;

		auto pos = gameObject.transform->GetWorldPosition();

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
#endif
	}
}//namespace