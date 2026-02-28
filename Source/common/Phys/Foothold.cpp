#include "Phys/Foothold.h"

#include <limits>
namespace sh::game
{
	Foothold::Foothold(GameObject& owner) :
		Component(owner)
	{

	}
	SH_USER_API auto Foothold::Serialize() const -> core::Json
	{
		auto json = Super::Serialize();

		for (const auto& path : paths)
		{
			core::Json pathJson;
			for (const auto& point : path.points)
			{
				core::Json pointJson;
				pointJson["x"] = point.x;
				pointJson["y"] = point.y;
				pathJson["points"].push_back(std::move(pointJson));
			}
			pathJson["tag"] = path.tag;
			json["paths"].push_back(std::move(pathJson));
		}

		return json;
	}
	SH_USER_API void Foothold::Deserialize(const core::Json& json)
	{
		Super::Deserialize(json);

		if (json.contains("paths"))
		{
			paths.resize(json["paths"].size());
			int i = 0;
			for (const auto& pathJson : json["paths"])
			{
				Path& path = paths[i];
				if (pathJson.contains("points"))
				{
					path.points.resize(pathJson["points"].size());
					int j = 0;
					for (const auto& pointJson : pathJson["points"])
					{
						path.points[j].x = pointJson.value("x", 0.f);
						path.points[j].y = pointJson.value("y", 0.f);
						++j;
					}
				}
				if (pathJson.contains("tag"))
					path.tag = pathJson["tag"];
				++i;
			}
		}
	}
	SH_USER_API auto Foothold::GetPath(int idx) const -> const Path*
	{
		if (idx < 0 || idx >= paths.size())
			return nullptr;
		return &paths[idx];
	}
	SH_USER_API auto Foothold::GetExpectedFallContact(const game::Vec2& pos) const -> Contact
	{
		Contact contact{};
		float bestY = std::numeric_limits<float>::lowest();
		int idx = 0;
		for (const auto& path : paths)
		{
			for (int i = 1; i < path.points.size(); ++i)
			{
				const auto& p0 = path.points[i - 1];
				const auto& p1 = path.points[i];

				if (std::abs(p0.x - p1.x) < std::numeric_limits<float>::epsilon())
					continue;

				const float minX = std::min(p0.x, p1.x);
				const float maxX = std::max(p0.x, p1.x);

				if (minX <= pos.x && pos.x <= maxX)
				{
					const float grad = (p1.y - p0.y) / (p1.x - p0.x);
					const float cy = grad * (pos.x - p0.x) + p0.y;
					if (cy <= pos.y && cy > bestY)
					{
						bestY = cy;
						contact.pathIdx = idx;
						contact.point0 = i - 1;
						contact.point1 = i;
						contact.grad = grad;
						contact.pos.x = pos.x;
						contact.pos.y = cy;
						contact.tag = path.tag;
					}
				}
			}
			++idx;
		}
		return contact;
	}
}//namespace