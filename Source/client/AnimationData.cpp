#include "AnimationData.h"
#include "AnimationData.h"
#include "AnimationData.h"
#include "AnimationData.h"

namespace sh::game
{
	AnimationData::~AnimationData()
	{
	}
	SH_USER_API auto AnimationData::GetTexture(int idx) const -> render::Texture*
	{
		if (idx < 0 || idx >= textures.size())
			return nullptr;
		return textures[idx];
	}
	SH_USER_API auto AnimationData::GetDelay(int idx) const -> uint32_t
	{
		if (idx < 0 || idx >= delayMs.size())
			return 0;
		return delayMs[idx];
	}
	SH_USER_API auto AnimationData::GetSize() const -> std::size_t
	{
		return textures.size();
	}
}//namespace