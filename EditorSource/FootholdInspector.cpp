#include "FootholdInspector.h"

#include "Game/World.h"
#include "Game/ImGUImpl.h"
#include "Game/Component/Render/MeshRenderer.h"

#include "Editor/EditorResource.h"
namespace sh::editor
{
	FootholdInspector::FootholdInspector()
	{
	}
	FootholdInspector::~FootholdInspector()
	{
		DestroyAdjustObjects();
	}
	SH_EDIT_API void FootholdInspector::RenderUI(void* obj, int idx)
	{
		game::Foothold& foothold = *reinterpret_cast<game::Foothold*>(obj);
		ImGui::SetCurrentContext(foothold.world.GetUiContext().GetContext());
		
		ImGui::Text("Path");
		for (int i = 0; i < foothold.paths.size(); ++i)
		{
			if (ImGui::TreeNodeEx(fmt::format("path{}", i).c_str(), ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_OpenOnArrow))
			{
				auto& path = foothold.paths[i];
				for (int j = 0; j < path.points.size(); ++j)
				{
					auto& point = path.points[j];
					float v[2] = { point.x, point.y };
					if (ImGui::InputFloat2(fmt::format("##path{}point{}", i, j).c_str(), v))
					{
						point.x = v[0];
						point.y = v[1];
					}
					ImGui::SameLine();
					if (ImGui::Button(fmt::format("-##{}", j).c_str()))
					{
						path.points.erase(path.points.begin() + j);
						if (adjustPathIdx != -1)
						{
							DestroyAdjustObjects();
							CreateAdjustObjects(foothold);
						}
					}
				}
				if (ImGui::Button("+"))
				{
					path.points.emplace_back();
					if (adjustPathIdx != -1)
					{
						DestroyAdjustObjects();
						CreateAdjustObjects(foothold);
					}
				}
				ImGui::SameLine();
				if (ImGui::Button("Adjust"))
				{
					if (adjustPathIdx == i)
					{
						adjustPathIdx = -1;
						DestroyAdjustObjects();
					}
					else
					{
						adjustPathIdx = i;
						CreateAdjustObjects(foothold);
					}
				}
				ImGui::TreePop();
			}
		}
		if (ImGui::Button("+"))
		{
			foothold.paths.emplace_back();
		}
		ImGui::Text("Adjust scale");
		ImGui::InputFloat("##adjustScale", &adJustScale);

		AdjustUpdate(foothold);
	}
	SH_EDIT_API void FootholdInspector::AdjustUpdate(game::Foothold& foothold)
	{
		if (adjustPathIdx == -1)
			return;

		auto& path = foothold.paths[adjustPathIdx];

		for (int i = 0; i < pointTransforms.size(); ++i)
		{
			if (path.points.size() <= i)
				break;
			auto transform = pointTransforms[i];
			if (!core::IsValid(transform))
				continue;
			path.points[i].x = transform->position.x;
			path.points[i].y = transform->position.y;

			if (i != 0)
			{
				auto line = lines[i];
				if (!core::IsValid(line) || !core::IsValid(pointTransforms[i - 1]))
					continue;
				line->SetStart({ transform->position.x, transform->position.y, transform->position.z });
				line->SetEnd({ pointTransforms[i - 1]->position.x, pointTransforms[i - 1]->position.y, pointTransforms[i - 1]->position.z });
			}
		}
	}
	void FootholdInspector::CreateAdjustObjects(game::Foothold& foothold)
	{
		if (adjustPathIdx == -1)
			return;

		DestroyAdjustObjects();
		pointTransforms.clear();
		lines.clear();

		game::World& world = foothold.world;

		root = world.AddGameObject("Temp");
		root->bEditorOnly = true;
		root->bNotSave = true;
		root->hideInspector = true;
		int i = 0;
		pointTransforms.resize(foothold.paths[adjustPathIdx].points.size());
		for (auto& point : foothold.paths[adjustPathIdx].points)
		{
			auto pointObj = world.AddGameObject(fmt::format("point{}", i));
			pointObj->transform->SetWorldPosition({ point.x, point.y, 0.f });
			pointObj->transform->SetParent(root->transform);
			pointObj->transform->SetScale(adJustScale);
			pointObj->bEditorOnly = true;
			pointObj->bNotSave = true;
			pointObj->hideInspector = true;
			pointTransforms[i] = pointObj->transform;

			{
				auto renderer = pointObj->AddComponent<game::MeshRenderer>();
				renderer->SetMesh(EditorResource::GetInstance()->GetModel("SphereModel")->GetMeshes()[0]);
			}
			{
				auto renderer = root->AddComponent<game::LineRenderer>();
				lines.push_back(renderer);
			}
			++i;
		}
	}
	void FootholdInspector::DestroyAdjustObjects()
	{
		if (root.IsValid())
			root->Destroy();
		root.Reset();
	}
}//namespace