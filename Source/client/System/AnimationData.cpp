#include "System/AnimationData.h"

#include "Core/SObject.h"
#include "Core/GarbageCollection.h"

namespace sh::game
{
	AnimationData::~AnimationData()
	{
	}
	SH_USER_API auto AnimationData::Serialize() const -> core::Json
	{
		core::Json json = Super::Serialize();

		auto& data = json["AnimationData"];
		data["frames"] = core::Json::array();
		data["bLoop"] = bLoop;
		for (const auto& frame : frames)
		{
			core::Json frameJson;
			if (core::IsValid(frame.texture))
				frameJson["texture"] = frame.texture->GetUUID().ToString();
			else
				frameJson["texture"] = core::UUID::GenerateEmptyUUID().ToString();
			frameJson["delayMs"] = frame.delayMs;
			frameJson["pos"] = { frame.pos.x, frame.pos.y };
			frameJson["a0"] = frame.a0;
			frameJson["a1"] = frame.a1;

			data["frames"].push_back(std::move(frameJson));
		}
		return json;
	}
	SH_USER_API void AnimationData::Deserialize(const core::Json& json)
	{
		Super::Deserialize(json);

		frames.clear();
		if (!json.contains("AnimationData"))
			return;

		const auto& data = json["AnimationData"];
		bLoop = data.value("bLoop", false);

		if (data.contains("frames"))
		{
			for (const auto& frameJson : data["frames"])
			{
				Frame frame;
				if (frameJson.contains("texture"))
				{
					const core::UUID texUUID{ frameJson["texture"].get_ref<const std::string&>() };
					frame.texture = static_cast<render::Texture*>(core::SObject::GetSObjectUsingResolver(texUUID));
				}
				frame.delayMs = frameJson.value("delayMs", 10u);
				if (frameJson.contains("pos") && frameJson["pos"].is_array() && frameJson["pos"].size() >= 2)
				{
					frame.pos.x = frameJson["pos"][0];
					frame.pos.y = frameJson["pos"][1];
				}
				frame.a0 = frameJson.value("a0", (uint8_t)255);
				frame.a1 = frameJson.value("a1", (uint8_t)255);

				frames.push_back(frame);
			}
		}
	}
	SH_USER_API auto AnimationData::GetFrame(int idx) const -> const Frame*
	{
		if (idx < 0 || idx >= frames.size())
			return nullptr;
		return &frames[idx];
	}
	SH_USER_API auto AnimationData::GetTexture(int idx) const -> render::Texture*
	{
		if (idx < 0 || idx >= frames.size())
			return nullptr;
		return frames[idx].texture;
	}
	SH_USER_API auto AnimationData::GetDelay(int idx) const -> uint32_t
	{
		if (idx < 0 || idx >= frames.size())
			return 0;
		return frames[idx].delayMs;
	}
	SH_USER_API auto AnimationData::GetPos(int idx) const -> const Vec2&
	{
		static Vec2 zero{ 0.f, 0.f };
		if (idx < 0 || idx >= frames.size())
			return zero;
		return frames[idx].pos;
	}
	SH_USER_API auto AnimationData::GetAlphaSrc(int idx) const -> uint8_t
	{
		if (idx < 0 || idx >= frames.size())
			return 255;
		return frames[idx].a0;
	}
	SH_USER_API auto AnimationData::GetAlphaDst(int idx) const -> uint8_t
	{
		if (idx < 0 || idx >= frames.size())
			return 255;
		return frames[idx].a1;
	}
	SH_USER_API void AnimationData::Frame::PushReferenceObjects(core::GarbageCollection& gc)
	{
		gc.PushReferenceObject(texture);
	}
}//namespace
