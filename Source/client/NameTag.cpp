#include "NameTag.h"

#include "Core/Util.h"

#include "Game/World.h"
#include "Game/Asset/FontGenerator.h"
namespace sh::game
{
	NameTag::NameTag(GameObject& owner) :
		Component(owner)
	{
	}
	SH_USER_API void NameTag::Start()
	{
	}
	SH_USER_API void NameTag::BeginUpdate()
	{
		if (bRequireNewFont)
		{
			CreateFont();
			SetBackgroundScale();
		}
	}
	SH_USER_API void NameTag::OnPropertyChanged(const core::reflection::Property& prop)
	{
		Super::OnPropertyChanged(prop);
		if (prop.GetName() == core::Util::ConstexprHash("nameStr"))
			Setup();
	}
	SH_USER_API void NameTag::SetNameStr(const std::string& name)
	{
		nameStr = name;
		Setup();
	}
	void NameTag::Setup()
	{
		const char* start = nameStr.data();
		const char* end = nameStr.data() + nameStr.size();

		while (start != end)
		{
			uint32_t unicode;
			start = core::Util::UTF8ToUnicode(start, end, unicode);
			if (unicodeSet.find(unicode) == unicodeSet.end())
				bRequireNewFont = true;
			unicodeSet.insert(unicode);
		}

		if (!bRequireNewFont)
		{
			if (textRenderer != nullptr)
				textRenderer->SetText(nameStr);
			SetBackgroundScale();
		}
	}
	void NameTag::CreateFont()
	{
		if (rawFont == nullptr)
			return;

		std::vector<uint32_t> unicodes(unicodeSet.begin(), unicodeSet.end());
		FontGenerator::Options opt{};
		opt.atlasW = 256;
		opt.atlasH = 256;
		font = FontGenerator::GenerateFont(*world.renderer.GetContext(), rawFont->data, unicodes, opt);
		assert(font != nullptr);
		textRenderer->SetFont(font);
		textRenderer->SetText(nameStr);

		bRequireNewFont = false;
	}
	auto NameTag::GetTextWidth() const -> float
	{
		float length = 0.f;
		const char* start = nameStr.data();
		const char* end = nameStr.data() + nameStr.size();
		while (start != end)
		{
			uint32_t unicode;
			start = core::Util::UTF8ToUnicode(start, end, unicode);

			auto glyphPtr = font->GetGlyph(unicode);
			if (glyphPtr != nullptr)
				length += glyphPtr->advance;
		}
		if (textRenderer != nullptr)
			length *= textRenderer->gameObject.transform->GetWorldScale().x;
		return length;
	}
	void NameTag::SetBackgroundScale()
	{
		float width = GetTextWidth() * 0.5f;
		if (backgroundRenderer != nullptr)
		{
			auto scale = backgroundRenderer->gameObject.transform->scale;
			backgroundRenderer->gameObject.transform->SetScale(width, scale.y, scale.z);
		}
		if (textRenderer != nullptr)
		{
			auto pos = textRenderer->gameObject.transform->position;
			textRenderer->gameObject.transform->SetPosition(-width, pos.y, pos.z);
		}
	}
}//namespace