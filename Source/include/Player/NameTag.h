#pragma once
#include "Export.h"

#include "Core/SContainer.hpp"
#include "Core/Observer.hpp"

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

		SH_USER_API void OnDestroy() override;
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

		/// @brief 모든 NameTag가 없어지면 같이 제거 되는 공용 폰트 관리자
		class GlobalFont
		{
		public:
			~GlobalFont();
			void SetFont(render::Font& font);
			auto GetFont() -> render::Font* { return font; }
			static auto GetInstance() -> std::shared_ptr<GlobalFont>;
		private:
			GlobalFont() = default;
		public:
			std::unordered_set<uint32_t> unicodeSet;
			core::Observer<false, render::Font*> onUpdated;
		private:
			render::Font* font = nullptr;
			
			inline static std::weak_ptr<GlobalFont> instance;
		};

		std::shared_ptr<GlobalFont> globalFont;
		core::Observer<false, render::Font*>::Listener onUpdatedListener;

		inline static bool bRequireNewFont = false;
	};
}//namespace