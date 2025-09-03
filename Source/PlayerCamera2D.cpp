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
		world.renderer.GetWindow().SetSize(1920, 1080);

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

		const Vec3& camPos = targetCamera->gameObject.transform->GetWorldPosition();
		Vec3 lookPos = targetCamera->GetLookPos();
		lookPos.x = camPos.x;
		lookPos.y = camPos.y;
		targetCamera->SetLookPos(lookPos);

		const float w = gameObject.world.renderer.GetWidth();
		const float h = gameObject.world.renderer.GetHeight();

		float dis = h * 0.005f;
		Vec3 pos = camPos;
		pos.z = lookPos.z + dis;

		targetCamera->gameObject.transform->SetWorldPosition(pos);
	}
	SH_USER_API void PlayerCamera2D::Update()
	{
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
	}
	SH_USER_API void PlayerCamera2D::LateUpdate()
	{
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
		float width = (world.renderer.GetWidth() >> 1) / 100.f;
		float height = (world.renderer.GetHeight() >> 1) / 100.f;

		float x = gameObject.transform->GetWorldPosition().x;
		float y = gameObject.transform->GetWorldPosition().y;
		float z = gameObject.transform->GetWorldPosition().z;

		float minX = std::min(this->minX + width, this->maxX - width);
		float maxX = std::max(this->minX + width, this->maxX - width);

		if (x >= minX && x <= maxX)
			x = SmoothDamp(x, centerX, velocityX, smoothTime, world.deltaTime);

		if (y >= minY && y <= maxY)
			y = SmoothDamp(y, centerY, velocityY, smoothTime, world.deltaTime);

		x = std::clamp(x, minX, maxX);
		//y = std::clamp(y, minY, maxY);

		gameObject.transform->SetWorldPosition({ x, y, z });
	}
}//namespace