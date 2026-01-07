#pragma once
#include "Export.h"

#include "Game/Component/Component.h"
#include "Game/Prefab.h"
#include "Game/TextObject.h"

#include "Render/Texture.h"

#include <vector>
#include <unordered_map>
namespace sh::game
{
	/// @brief 에셋 번들에 포함 되게 하기 위한 더미 컴포넌트
	class AssetExportHelper : public Component
	{
		COMPONENT(AssetExportHelper, "user")
	public:
		SH_USER_API AssetExportHelper(GameObject& owner);
	private:
		PROPERTY(text)
		TextObject* text = nullptr;
		PROPERTY(texture)
		render::Texture* texture = nullptr;
		PROPERTY(prefab)
		Prefab* prefab = nullptr;
	};
}//namespace