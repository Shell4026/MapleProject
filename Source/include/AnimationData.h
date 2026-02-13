#pragma once
#include "Export.h"

#include "Render/Texture.h"

#include "Game/ScriptableObject.h"
#include "Game/Vector.h"

#include <vector>
namespace sh::editor
{
	class AnimationDataInspector;
}
namespace sh::game
{
	class AnimationData : public ScriptableObject
	{
		SRPO(AnimationData)
		friend editor::AnimationDataInspector;
	public:
		SH_USER_API ~AnimationData();

		SH_USER_API auto GetTexture(int idx) const -> render::Texture*;
		SH_USER_API auto GetDelay(int idx) const -> uint32_t;
		SH_USER_API auto GetSize() const -> std::size_t;
		SH_USER_API auto GetPos() const -> const Vec2& { return pos; }
		SH_USER_API auto IsLoop() const -> bool { return bLoop; }
	private:
		PROPERTY(textures)
		std::vector<render::Texture*> textures;
		PROPERTY(delayMs)
		std::vector<uint32_t> delayMs;
		PROPERTY(pos)
		Vec2 pos{ 0.f, 0.f };
		PROPERTY(bLoop)
		bool bLoop = false;
	};
}//namespace