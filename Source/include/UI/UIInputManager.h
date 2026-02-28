#pragma once
#include "Export.h"

#include "Core/GCObject.h"

#include "Game/Component/Component.h"
#include "Game/Component/Transform.h"

#include <vector>
#include <memory>
#include <cstdint>
namespace sh::game
{
	class UIRect;
	class UIInputManager : public Component
	{
		COMPONENT(UIInputManager, "user")
	public:
		SH_USER_API UIInputManager(GameObject& owner);

		SH_USER_API void OnDestroy() override;
		SH_USER_API void Awake() override;
		SH_USER_API void Start() override;
		SH_USER_API void BeginUpdate() override;
		SH_USER_API void Update() override;

		SH_USER_API static auto GetInstance() -> UIInputManager&;
		SH_USER_API static auto IsValidInstance() -> bool;

		SH_USER_API void BuildRectList(const UIRect& rect);
	private:
		void BuildDirtyRectLists();
		struct RectNode : core::GCObject
		{
			UIRect* rect = nullptr;
			RectNode* parent = nullptr;
			std::vector<std::unique_ptr<RectNode>> childs;

			SH_USER_API void PushReferenceObjects(core::GarbageCollection& gc) override;
		};

		static auto FindRoot(const UIRect& rect) -> UIRect&;
		static auto CreateRectNode(const UIRect& rect, RectNode* parent) -> std::unique_ptr<RectNode>;
		static auto FindRect(const Transform& transform) -> UIRect*;
	private:
		static UIInputManager* instance;

		std::vector<std::unique_ptr<RectNode>> rootNodes;
		PROPERTY(dirtyRoots, core::PropertyOption::sobjPtr, core::PropertyOption::invisible)
		std::unordered_set<UIRect*> dirtyRoots;
		uint32_t nullptrCount = 0;

		bool bChangedRects = false;
	};
}//namespace