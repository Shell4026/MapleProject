#pragma once
#include "Export.h"
#include "Phys/CollisionTag.hpp"

#include "Game/Component/Component.h"
#include "Game/Vector.h"

namespace sh::editor
{
	class FootholdInspector;
}
namespace sh::game
{
	class Foothold : public Component
	{
		COMPONENT(Foothold, "user")
		friend editor::FootholdInspector;
	public:
		struct Path
		{
			std::vector<Vec2> points;
			phys::Tagbit tag = tag::groundTag;
		};
		struct Contact
		{
			int pathIdx = -1;
			int point0 = -1;
			int point1 = -1;
			float grad = 0.f;
			Vec2 pos;
			phys::Tagbit tag;
		};
	public:
		SH_USER_API Foothold(GameObject& owner);

		SH_USER_API auto Serialize() const -> core::Json override;
		SH_USER_API void Deserialize(const core::Json& json) override;

		SH_USER_API auto GetPath(int idx) const -> const Path*;
		SH_USER_API auto GetExpectedFallContact(const game::Vec2& pos) const -> Contact;
	private:
		std::vector<Path> paths;
	};
}//namespace