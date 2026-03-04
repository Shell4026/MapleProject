#pragma once
#include "Export.h"

#include "Render/Texture.h"

#include "Core/GCObject.h"

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
		struct Frame : core::GCObject
		{
			render::Texture* texture = nullptr;
			uint32_t delayMs = 10;
			Vec2 pos{ 0.f, 0.f };
			uint8_t a0 = 255;
			uint8_t a1 = 255;

			SH_USER_API void PushReferenceObjects(core::GarbageCollection& gc) override;
		};
	public:
		SH_USER_API ~AnimationData();

		SH_USER_API auto Serialize() const -> core::Json override;
		SH_USER_API void Deserialize(const core::Json& json) override;

		SH_USER_API auto GetFrame(int idx) const -> const Frame*;
		SH_USER_API auto GetTexture(int idx) const -> render::Texture*;
		SH_USER_API auto GetDelay(int idx) const -> uint32_t;
		SH_USER_API auto GetPos(int idx) const -> const Vec2&;
		SH_USER_API auto GetAlphaSrc(int idx) const -> uint8_t;
		SH_USER_API auto GetAlphaDst(int idx) const -> uint8_t;

		SH_USER_API auto GetSize() const -> std::size_t { return frames.size(); }
		SH_USER_API auto IsLoop() const -> bool { return bLoop; }
	private:
		std::vector<Frame> frames;
		PROPERTY(bLoop)
		bool bLoop = false;
	};
}//namespace
