#include "Item/Item.h"
#include "Phys/CollisionTag.hpp"
#include "Phys/FootholdMovement.h"

#include "Game/GameObject.h"

// 서버 사이드
namespace sh::game
{
	Item::Item(GameObject& owner) :
		Component(owner)
	{
	}
	SH_USER_API void Item::Awake()
	{
		if (renderer == nullptr || rb == nullptr)
		{
			SH_ERROR("Invaild item properties!");
			return;
		}
		if (movement == nullptr)
			SH_ERROR("movement is nullptr!");
	}
}//namespace