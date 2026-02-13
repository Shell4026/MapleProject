#include "AnimationDataInspector.h"

#include <limits>
namespace sh::editor
{
	SH_EDIT_API void AnimationDataInspector::RenderUI(void* obj, int idx)
	{
		CustomInspector::RenderUI(obj, idx);

		game::AnimationData& anim = *reinterpret_cast<game::AnimationData*>(obj);
		if (anim.IsPendingKill())
			return;

		if (anim.textures.size() != anim.delayMs.size())
			anim.delayMs.resize(anim.textures.size());

		bool bChanged = false;

		ImGui::Text("textures/delay(ms)");
		for (int i = 0; i < anim.textures.size(); ++i)
		{
			auto& texPtr = anim.textures[i];
			uint32_t& delayMs = anim.delayMs[i];
			int delay = delayMs;

			std::string name = core::IsValid(texPtr) ? texPtr->GetName().ToString() : "None";
			if (ImGui::Button(name.c_str()))
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
				delayMs = std::clamp(delay, 10, std::numeric_limits<int>::max());
				bChanged = true;
			}

			ImGui::SameLine();

			if (ImGui::Button(fmt::format("-##{}", i).c_str()))
			{
				anim.textures.erase(anim.textures.begin() + i);
				anim.delayMs.erase(anim.delayMs.begin() + i);
				AssetDatabase::GetInstance()->SetDirty(&anim);
				AssetDatabase::GetInstance()->SaveAllAssets();
				break;
			}
		}

		ImGui::Separator();
		if (ImGui::Button("Add"))
		{
			anim.textures.push_back(nullptr);
			anim.delayMs.push_back(10);
			bChanged = true;
		}

		ImGui::Text("Position");
		float pos[2] = { anim.pos.x, anim.pos.y };
		if (ImGui::InputFloat2("##position", pos))
		{
			anim.pos.x = pos[0];
			anim.pos.y = pos[1];
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