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
	SH_USER_API void PlayerMovement::Start()
	{
		SetFoothold(*player->GetCurrentWorld()->GetFoothold());
		Super::Start();
	}
}//namespace