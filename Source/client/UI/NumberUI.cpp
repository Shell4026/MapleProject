#include "UI/NumberUI.h"

#include "Game/GameObject.h"
namespace sh::game
{
	NumberUI::NumberUI(GameObject& owner) :
		Component(owner),
		bDirty(true)
	{
	}
	SH_USER_API void NumberUI::Update()
	{
		if (bDirty)
			UpdateRenderers();
	}
	SH_USER_API void NumberUI::SetNumber(int num)
	{
		number = std::to_string(num);
		bDirty = true;
	}
	void NumberUI::UpdateRenderers()
	{
		for (auto renderer : renderers)
			renderer->gameObject.SetActive(false);
		for (int i = 0; i < renderers.size(); ++i)
		{
			if (number.size() < i)
				break;
			const int n = number[i] - '0';
			if (numTexs.size() <= n)
				continue;
		
			MeshRenderer* renderer = renderers[i];
			renderer->gameObject.SetActive(true);
			renderer->GetMaterialPropertyBlock()->SetProperty("tex", numTexs[n]);
			renderer->UpdatePropertyBlockData();
		}

		bDirty = false;
	}
}//namespace