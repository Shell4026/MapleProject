#include "AnimationDataInspector.h"

#include <limits>
namespace sh::editor
{
	SH_EDIT_API void AnimationDataInspector::RenderUI(const std::vector<core::SObject*>& objs, int idx)
	{
		CustomInspector::RenderUI(objs, idx);

		game::AnimationData& anim = *reinterpret_cast<game::AnimationData*>(objs.back());
		if (anim.IsPendingKill())
			return;

		bool bChanged = false;

		ImGui::Text("frames/delay/x/y/a0/a1");
		for (int i = 0; i < anim.frames.size(); ++i)
		{
			auto& frame = anim.frames[i];
			auto& texPtr = frame.texture;
			int delay = frame.delayMs;
			float pos[2] = { frame.pos.x, frame.pos.y };
			int alpha[2] = { frame.a0, frame.a1 };

			std::string name = core::IsValid(texPtr) ? texPtr->GetName().ToString() : "None";
			if (ImGui::Button(fmt::format("{}##tex{}", name, i).c_str()))
			{
			}
			if (ImGui::BeginDragDropTarget())
			{
				const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(std::string{ render::Texture::GetStaticType().type.name }.c_str());
				if (payload != nullptr)
				{
					texPtr = static_cast<render::Texture*>(*reinterpret_cast<core::SObject**>(payload->Data));
					bChanged = true;
				}
			}
			ImGui::SameLine();

			ImGui::SetNextItemWidth(40);
			if (ImGui::InputInt(fmt::format("##delay{}", i).c_str(), &delay, 0))
			{
				frame.delayMs = std::clamp(delay, 10, std::numeric_limits<int>::max());
				bChanged = true;
			}

			ImGui::SameLine();
			ImGui::SetNextItemWidth(130);
			if (ImGui::InputFloat2(fmt::format("##pos{}", i).c_str(), pos))
			{
				frame.pos.x = pos[0];
				frame.pos.y = pos[1];
				bChanged = true;
			}

			ImGui::SameLine();
			ImGui::SetNextItemWidth(130);
			if (ImGui::InputInt2(fmt::format("##alpha{}", i).c_str(), alpha))
			{
				frame.a0 = std::clamp(alpha[0], 0, 255);
				frame.a1 = std::clamp(alpha[1], 0, 255);
				bChanged = true;
			}

			ImGui::SameLine();

			if (ImGui::Button(fmt::format("-##{}", i).c_str()))
			{
				anim.frames.erase(anim.frames.begin() + i);
				AssetDatabase::GetInstance()->SetDirty(&anim);
				AssetDatabase::GetInstance()->SaveAllAssets();
				break;
			}
		}

		ImGui::Separator();
		if (ImGui::Button("Add"))
		{
			anim.frames.push_back({});
			bChanged = true;
		}

		if (ImGui::Checkbox("Loop", &anim.bLoop))
			bChanged = true;

		if (bChanged)
		{
			AssetDatabase::GetInstance()->SetDirty(&anim);
			AssetDatabase::GetInstance()->SaveAllAssets();
		}
	}
}//namespace
