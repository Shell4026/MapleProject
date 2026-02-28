#include "Item/Item.h"
#include "Phys/CollisionTag.hpp"
#include "Phys/FootholdMovement.h"
#include "World/MapleWorld.h"

#include "Game/GameObject.h"

// 클라 사이드
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
		if (texture != nullptr)
		{
			renderer->GetMaterialPropertyBlock()->SetProperty("tex", texture);
			renderer->UpdatePropertyBlockData();
		}
	}
	SH_USER_API void Item::SetTexture(const render::Texture* texture)
	{
		this->texture = texture;
		if (renderer == nullptr)
			return;
		renderer->GetMaterialPropertyBlock()->SetProperty("tex", texture);
		renderer->UpdatePropertyBlockData();
	}
}//namespace