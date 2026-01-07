#include "AI/BasicAI.h"
#include "Mob/Mob.h"

namespace sh::game
{
	BasicAI::BasicAI(GameObject& owner) :
		AIStrategy(owner)
	{
	}
	SH_USER_API void BasicAI::Run(Mob& mob)
	{
	}
	SH_USER_API void BasicAI::OnAttacked(Player& player)
	{
		
	}
	SH_USER_API auto BasicAI::GetState() const -> uint32_t
	{
		return 0;
	}
	SH_USER_API void BasicAI::Reset()
	{
	}
}//namespace