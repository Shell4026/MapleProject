#pragma once
#include "Export.h"

#include "Game/Component/Component.h"
#include "Game/Component/Render/MeshRenderer.h"

#include <vector>
namespace sh::game
{
	class NumberUI : public Component
	{
		COMPONENT(NumberUI, "user")
	public:
		SH_USER_API NumberUI(GameObject& owner);

		SH_USER_API void Update() override;

		SH_USER_API void SetNumber(int num);
		SH_USER_API auto GetNumber() const -> int { return std::stoi(number); }
	private:
		void UpdateRenderers();
	private:
		PROPERTY(numTexs)
		std::vector<render::Texture*> numTexs;
		PROPERTY(renderers)
		std::vector<MeshRenderer*> renderers;

		std::string number = "0";

		bool bDirty = false;
	};
}//namespace