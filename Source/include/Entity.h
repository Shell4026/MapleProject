#pragma once
#include "Game/Component/Component.h"

namespace sh::game
{
	class Entity : public Component
	{
		SCLASS(Entity)
	public:
		enum class Type
		{
			Player,
			Mob
		};
	public:
		Entity(GameObject& owner) :
			Component(owner)
		{
		}
		virtual auto GetEntityType() const -> Type = 0;
	};
}//namespace