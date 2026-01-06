#include "Item/Item.h"
#include "CollisionTag.hpp"

#include "Game/GameObject.h"

namespace sh::game
{
	Item::Item(GameObject& owner) :
		Component(owner)
	{
	}
	SH_USER_API void Item::Awake()
	{
		if (renderer == nullptr || rb == nullptr || trigger == nullptr)
		{
			SH_ERROR("Invaild item properties!");
			return;
		}
		if (texture != nullptr)
		{
			renderer->GetMaterialPropertyBlock()->SetProperty("tex", texture);
			renderer->UpdatePropertyBlockData();
		}

		rb->GetCollider()->SetCollisionTag(tag::entityTag);
		rb->GetCollider()->SetAllowCollisions(tag::groundTag);

		trigger->GetCollider()->SetCollisionTag(tag::itemTag);
		trigger->GetCollider()->SetAllowCollisions(tag::entityTag);
	}
	SH_USER_API void Item::OnDisable()
	{
		rb->SetLinearVelocity({ 0.f, 0.f, 0.f });
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