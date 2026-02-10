#pragma once
#include "ExportEditor.h"
#include "Physics/Foothold.h"

#include "Core/SContainer.hpp"

#include "Game/GameObject.h"
#include "Game/Component/Transform.h"
#include "Game/Component/Render/LineRenderer.h"

#include "Editor/UI/CustomInspector.h"
namespace sh::editor
{
	class FootholdInspector : public ICustomInspector
	{
		INSPECTOR(FootholdInspector, game::Foothold)
	public:
		SH_EDIT_API FootholdInspector();
		SH_EDIT_API ~FootholdInspector();

		SH_EDIT_API void RenderUI(void* obj, int idx) override;
		SH_EDIT_API void AdjustUpdate(game::Foothold& foothold);
	private:
		void CreateAdjustObjects(game::Foothold& foothold);
		void DestroyAdjustObjects();
	private:
		core::SObjWeakPtr<game::GameObject> root;
		core::SVector<game::Transform*> pointTransforms;
		core::SVector<game::LineRenderer*> lines;
		float adJustScale = 0.2f;
		int adjustPathIdx = -1;
		bool bAdjustMode = false;
	};
}//namespace