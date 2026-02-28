#pragma once
#include "Physics/CollisionTag.hpp"

namespace sh::game::tag
{
	constexpr phys::Tag groundTag = phys::Tag::Tag1;
	constexpr phys::Tag entityTag = phys::Tag::Tag2;
	constexpr phys::Tag mobHitboxTag = phys::Tag::Tag3;
	constexpr phys::Tag projectileHitboxTag = phys::Tag::Tag4;
	constexpr phys::Tag itemTag = phys::Tag::Tag5;
	constexpr phys::Tag playerTag = phys::Tag::Tag6;
	constexpr phys::Tag portalTag = phys::Tag::Tag7;
	constexpr phys::Tagbit playerInteractionTagMask = itemTag | portalTag;
	constexpr phys::Tagbit entityTagMask = groundTag;
}//namespace
