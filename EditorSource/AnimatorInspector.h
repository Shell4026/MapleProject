#pragma once
#include "ExportEditor.h"
#include "System/Animator.h"

#include "Editor/UI/CustomInspector.h"
namespace sh::editor
{
	class AnimatorInspector : public CustomInspector
	{
		INSPECTOR(AnimatorInspector, game::Animator)
	public:
		SH_EDIT_API void RenderUI(const std::vector<core::SObject*>& objs, int idx) override;
	};
}//namespace