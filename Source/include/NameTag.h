#pragma once
#include "Export.h"

#include "Core/SContainer.hpp"

#include "Game/Component/Component.h"
#include "Game/Component/Render/MeshRenderer.h"
#include "Game/Component/Render/TextRenderer.h"
#include "Game/BinaryObject.h"

#include "Render/Font.h"

#include <vector>
#include <unordered_set>
#include <string>
#include <memory>

namespace sh::game
{
	class NameTag : public Component
	{
		COMPONENT(NameTag, "user")
	public:
		SH_USER_API NameTag(GameObject& owner);

		SH_USER_API void Start() override;
		SH_USER_API void BeginUpdate() override;
		SH_USER_API void OnPropertyChanged(const core::reflection::Property& prop) override;

		SH_USER_API void SetNameStr(const std::string& name);

		SH_USER_API auto GetNameStr() const -> const std::string& { return nameStr; }
	private:
		void Setup();
		void CreateFont();
		auto GetTextWidth() const -> float;
		void SetBackgroundScale();
	private:
		PROPERTY(nameStr)
		std::string nameStr;
		PROPERTY(backgroundRenderer)
		MeshRenderer* backgroundRenderer = nullptr;
		PROPERTY(textRenderer)
		TextRenderer* textRenderer = nullptr;
		PROPERTY(rawFont)
		BinaryObject* rawFont = nullptr;
		PROPERTY(font, core::PropertyOption::invisible)
		render::Font* font = nullptr;

		struct GlobalFont
		{

		};

		bool bRequireNewFont = false;
		std::unordered_set<uint32_t> unicodeSet;
	};
}//namespace