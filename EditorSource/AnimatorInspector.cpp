#include "AnimatorInspector.h"

#include "Editor/UI/Inspector.h"

#include <limits>
namespace sh::editor
{
	SH_EDIT_API void AnimatorInspector::RenderUI(void* obj, int idx)
	{
		CustomInspector::RenderUI(obj, idx);

		game::Animator& animator = *reinterpret_cast<game::Animator*>(obj);
		if (animator.IsPendingKill())
			return;

		ImGui::Text("animation/condition");
		for (int i = 0; i < animator.anims.size(); ++i)
		{
			auto& animPtr = animator.anims[i].anim;
			int& condition = animator.anims[i].condition;

			std::string name = core::IsValid(animPtr) ? animPtr->GetName().ToString() : "None";
			if (ImGui::Button(name.c_str()))
			{
			}
			if (ImGui::BeginDragDropTarget())
			{
				const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(std::string{ game::AnimationData::GetStaticType().type.name }.c_str());
				if (payload != nullptr)
				{
					animPtr = static_cast<game::AnimationData*>(*reinterpret_cast<core::SObject**>(payload->Data));
				}
			}
			ImGui::SameLine();

			ImGui::SetNextItemWidth(40);
			ImGui::InputInt(fmt::format("##delay{}", i).c_str(), &condition, 0);

			ImGui::SameLine();

			if (ImGui::Button(fmt::format("-##{}", i).c_str()))
			{
				animator.anims.erase(animator.anims.begin() + i);
				break;
			}
		}
		ImGui::Separator();
		if (ImGui::Button("Add"))
		{
			animator.anims.emplace_back();
		}

		Inspector::RenderProperties(animator.GetType(), animator, idx);
	}
}//namespace