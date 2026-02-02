#include "NPC/NPC.h"
#include "NPC/NPCInfo.hpp"
#include "NPC/NPCDB.h"

#include "Game/Input.h"
#include "Game/World.h"

namespace sh::game
{
	NPC::NPC(GameObject& owner) :
		UIRect(owner)
	{
	}
	SH_USER_API void NPC::Awake()
	{
		Super::Awake();
		info = NPCDB::GetNPCInfo(id);
	}
	SH_USER_API void NPC::OnHover()
	{
		if (Input::GetMouseReleased(Input::MouseType::Left))
			ShowDialog();
	}
	void NPC::ShowDialog()
	{
		if (dialogPrefab == nullptr || info == nullptr || DialogUI::IsOpen())
			return;

		GameObject* dialogObjPtr = dialogPrefab->AddToWorld(world);
		DialogUI* uiPtr = dialogObjPtr->GetComponent<DialogUI>();
		if (uiPtr == nullptr || !uiPtr->IsValid())
		{
			SH_ERROR_FORMAT("Invalid dialog prefab! ({})", dialogPrefab->GetUUID().ToString());
			dialogObjPtr->Destroy();
			dialogPrefab = nullptr;
			return;
		}
		dialogObjPtr->transform->SetWorldPosition(world.renderer.GetWidth() * 0.005f, world.renderer.GetHeight() * 0.005f, dialogObjPtr->transform->GetWorldPosition().z);
		uiPtr->SetText(info->script);
		if (npcTexture != nullptr)
			uiPtr->SetNpcTexture(*npcTexture);
	}
}//namespace