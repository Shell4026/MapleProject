#include "Export.h"
#include "UI/UIRect.h"
#include "UI/DialogUI.h"

#include "Game/Prefab.h"
#include "Game/Component/Render/MeshRenderer.h"

#include "Render/Texture.h"

#include <cstdint>
namespace sh::game
{
	class NPCInfo;

	class NPC : public UIRect
	{
		COMPONENT(NPC, "user")
	public:
		SH_USER_API NPC(GameObject& owner);

		SH_USER_API void Awake() override;
		SH_USER_API void OnHover() override;

		SH_USER_API auto GetNPCId() const -> uint32_t { return id; }
	private:
		void ShowDialog();
	private:
		PROPERTY(id)
		uint32_t id = 0;
		PROPERTY(dialogPrefab)
		Prefab* dialogPrefab = nullptr;
#if !SH_SERVER
		PROPERTY(npcTexture)
		render::Texture* npcTexture = nullptr;
#endif

		const NPCInfo* info = nullptr;
	};
}//namespace