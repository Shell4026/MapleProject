#include "SkillConditionInspector.h"

#include "Editor/UI/Inspector.h"

namespace sh::editor
{
	SH_EDIT_API void SkillConditionInspector::RenderUI(const std::vector<core::SObject*>& objs, int idx)
	{
		CustomInspector::RenderUI(objs, idx);

		game::SkillCondition& condition = *reinterpret_cast<game::SkillCondition*>(objs.back());
		if (condition.IsPendingKill())
			return;

		bool bChanged = false;
		const game::SkillCondition::Type curType = condition.GetConditionType();
		const char* preview = game::SkillCondition::TypeToString(curType);
		if (ImGui::BeginCombo("Type", preview))
		{
			for (int i = 0; i <= static_cast<int>(game::SkillCondition::Type::PreviousSkillIn); ++i)
			{
				const auto type = static_cast<game::SkillCondition::Type>(i);
				const bool bSelected = type == curType;
				if (ImGui::Selectable(game::SkillCondition::TypeToString(type), bSelected))
				{
					condition.type = static_cast<uint32_t>(type);
					bChanged = true;
				}
				if (bSelected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}

		Inspector::RenderProperties(condition.GetType(), condition, idx);

		if (bChanged)
		{
			AssetDatabase::GetInstance()->SetDirty(&condition);
			AssetDatabase::GetInstance()->SaveAllAssets();
		}
	}
}//namespace
