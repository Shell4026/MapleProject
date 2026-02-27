#pragma once
#include "ExportEditor.h"
#include "AnimationData.h"

#include "Editor/UI/CustomInspector.h"
namespace sh::editor
{
	class AnimationDataInspector : public CustomInspector
	{
		INSPECTOR(AnimationDataInspector, game::AnimationData)
	public:
		SH_EDIT_API void RenderUI(void* obj, int idx) override;
	};
}//namespace