#include "NameTag.h"

#include "Core/Util.h"
#include "Core/GarbageCollection.h"

#include "Game/World.h"
#include "Game/Asset/FontGenerator.h"
namespace sh::game
{
	NameTag::NameTag(GameObject& owner) :
		Component(owner)
	{
		globalFont = GlobalFont::GetInstance();
		onUpdatedListener.SetCallback(
			[this](render::Font* font)
			{
				textRenderer->SetFont(font);
				textRenderer->SetText(nameStr);
				SetBackgroundScale();
			}
		);
		globalFont->onUpdated.Register(onUpdatedListener);
	}
	SH_USER_API void NameTag::OnDestroy()
	{
		globalFont.reset();
		Super::OnDestroy();
	}
	SH_USER_API void NameTag::Start()
	{
		Setup();
	}
	SH_USER_API void NameTag::BeginUpdate()
	{
		if (bRequireNewFont)
		{
			CreateFont();
			Setup();
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
			if (globalFont->unicodeSet.find(unicode) == globalFont->unicodeSet.end())
				bRequireNewFont = true;
			globalFont->unicodeSet.insert(unicode);
		}

		if (!bRequireNewFont)
		{
			if (textRenderer != nullptr)
			{
				textRenderer->SetFont(globalFont->GetFont());
				textRenderer->SetText(nameStr);
			}
			SetBackgroundScale();
		}
	}
	void NameTag::CreateFont()
	{
		if (rawFont == nullptr)
			return;

		std::vector<uint32_t> unicodes(globalFont->unicodeSet.begin(), globalFont->unicodeSet.end());
		FontGenerator::Options opt{};
		opt.atlasW = 256;
		opt.atlasH = 256;
		globalFont->SetFont(*FontGenerator::GenerateFont(*world.renderer.GetContext(), rawFont->data, unicodes, opt));

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

			auto glyphPtr = globalFont->GetFont()->GetGlyph(unicode);
			if (glyphPtr != nullptr)
				length += glyphPtr->advance;
		}
		if (textRenderer != nullptr)
			length *= textRenderer->gameObject.transform->GetWorldScale().x;
		return length;
	}
	void NameTag::SetBackgroundScale()
	{
		const float width = GetTextWidth();
		if (backgroundRenderer != nullptr)
		{
			auto scale = backgroundRenderer->gameObject.transform->scale;
			backgroundRenderer->gameObject.transform->SetScale(width, scale.y, scale.z);
		}
		if (textRenderer != nullptr)
		{
			auto pos = textRenderer->gameObject.transform->position;
			textRenderer->gameObject.transform->SetPosition(-width * 0.5f, pos.y, pos.z);
		}
	}

	NameTag::GlobalFont::~GlobalFont()
	{
		if (font != nullptr)
			font->Destroy();
		SH_INFO("~font");
	}
	void NameTag::GlobalFont::SetFont(render::Font& font)
	{
		if (this->font != nullptr)
			this->font->Destroy();
		this->font = &font;
		core::GarbageCollection::GetInstance()->SetRootSet(this->font);
		onUpdated.Notify(this->font);
	}
	auto NameTag::GlobalFont::GetInstance() -> std::shared_ptr<GlobalFont>
	{
		auto ptr = instance.lock();
		if (ptr == nullptr)
		{
			ptr = std::shared_ptr<GlobalFont>(new GlobalFont{});
			instance = ptr;
		}
		return ptr;
	}
}//namespace