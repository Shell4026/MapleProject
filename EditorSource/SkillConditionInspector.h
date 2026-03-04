#pragma once
#include "ExportEditor.h"
#include "Skill/SkillCondition.h"

#include "Editor/UI/CustomInspector.h"
namespace sh::editor
{
	class SkillConditionInspector : public CustomInspector
	{
		INSPECTOR(SkillConditionInspector, game::SkillCondition)
	public:
		SH_EDIT_API void RenderUI(const std::vector<core::SObject*>& objs, int idx) override;
	};
}//namespace
