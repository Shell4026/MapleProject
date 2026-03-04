#include "Player/PlayerMovement.h"
#include "World/MapleWorld.h"

#include "Game/World.h"
#include "Game/Input.h"

namespace sh::game
{
	// 공용 코드
	PlayerMovement::PlayerMovement(GameObject& owner) :
		FootholdMovement(owner)
	{
	}
	SH_USER_API void PlayerMovement::ApplySkillImpulse(float vx, float vy)
	{
		SetVelocity(vx, vy);
		skillMoveThisTick.bImpulse = true;
		skillMoveThisTick.impulse = { vx, vy };
	}
	SH_USER_API void PlayerMovement::ApplySkillTeleport(float dx, float dy)
	{
		const auto& pos = gameObject.transform->GetWorldPosition();
		gameObject.transform->SetWorldPosition(pos.x + dx, pos.y + dy, pos.z);
		gameObject.transform->UpdateMatrix();
		SetVelocity(0.f, 0.f);
		SetExpectedGround();

		skillMoveThisTick.bTeleport = true;
		skillMoveThisTick.teleportDelta.x += dx;
		skillMoveThisTick.teleportDelta.y += dy;
		skillMoveThisTick.bSetExpectedGround = true;
	}
	SH_USER_API void PlayerMovement::Start()
	{
		SetFoothold(*player->GetCurrentWorld()->GetFoothold());
		Super::Start();
	}
}//namespace
