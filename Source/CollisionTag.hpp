#pragma once

#include "Physics/CollisionTag.hpp"

namespace sh::game::tag
{
	constexpr phys::Tag groundTag = phys::Tag::Tag1;
	constexpr phys::Tag entityTag = phys::Tag::Tag2;
	constexpr phys::Tagbit entityTagMask = groundTag;
}//namespace