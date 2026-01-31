#include "UI/UIInputManager.h"
#include "UI/UIRect.h"

#include "Game/GameObject.h"
#include "Game/GameManager.h"

#include <limits>
#include <algorithm>
namespace sh::game
{
	UIInputManager* UIInputManager::instance = nullptr;

	UIInputManager::UIInputManager(GameObject& owner) :
		Component(owner)
	{
	}
	SH_USER_API void UIInputManager::OnDestroy()
	{
		if (instance == this)
			instance = nullptr;
		Super::OnDestroy();
	}
	SH_USER_API void UIInputManager::Awake()
	{
		GameManager::GetInstance()->SetImmortalObject(gameObject);
	}
	SH_USER_API void UIInputManager::Start()
	{
		if (instance == nullptr)
			instance = this;
		else
		{
			SH_ERROR("UIInputManager is already exist!");
			Destroy();
		}
	}
	SH_USER_API void UIInputManager::BeginUpdate()
	{
		BuildDirtyRectLists();
	}
	SH_USER_API void UIInputManager::Update()
	{
		RectNode* lastNode = nullptr;
		float z = std::numeric_limits<float>::lowest();
		for (auto& node : rootNodes)
		{
			if (node == nullptr)
				continue;

			if (!node->rect->IsContainsMouse())
				continue;

			float rootZ = node->rect->gameObject.transform->GetWorldPosition().z;
			if (rootZ < z)
				continue;
			z = rootZ;

			std::queue<RectNode*> bfs;
			for (auto& child : node->childs)
				bfs.push(child.get());

			lastNode = node.get();
			while (!bfs.empty())
			{
				RectNode* curNode = bfs.front();
				bfs.pop();
				const UIRect* rect = curNode->rect;
				if (rect->IsContainsMouse())
				{
					lastNode = curNode;
					for (auto& child : curNode->childs)
						bfs.push(child.get());
				}
			}
		}
		if (lastNode != nullptr)
			lastNode->rect->OnHover();
	}
	SH_USER_API auto UIInputManager::GetInstance() -> UIInputManager&
	{
		if (instance == nullptr)
		{
			auto& obj = GameManager::GetInstance()->CreateImmortalObject("UIInputManager");
			instance = obj.AddComponent<UIInputManager>();
		}
		return *instance;
	}
	void UIInputManager::BuildRectList(const UIRect& rect)
	{
		UIRect& root = FindRoot(rect);
		dirtyRoots.insert(&root);
	}
	void UIInputManager::BuildDirtyRectLists()
	{
		if (nullptrCount >= 10)
		{
			rootNodes.erase(std::remove(rootNodes.begin(), rootNodes.end(), nullptr), rootNodes.end());
			nullptrCount = 0;
		}
		
		for (auto root : dirtyRoots)
		{
			std::unique_ptr<RectNode> node = nullptr;
			if (!root->IsPendingKill())
				node = CreateRectNode(*root, nullptr);

			bool bReplaced = false;
			for (int i = 0; i < rootNodes.size(); ++i)
			{
				if (rootNodes[i] == nullptr)
					continue;

				if (rootNodes[i]->rect == root)
				{
					if (node == nullptr)
						++nullptrCount;
					rootNodes[i] = std::move(node); // 트리를 다시 생성
					bReplaced = true;
					break;
				}
			}
			if (!bReplaced && node != nullptr)
				rootNodes.push_back(std::move(node));
		}
		dirtyRoots.clear();
	}
	auto UIInputManager::FindRoot(const UIRect& rect) -> UIRect&
	{
		const Transform* cur = rect.gameObject.transform->GetParent();
		const UIRect* root = &rect;

		while (cur != nullptr)
		{
			for (const Component* component : cur->gameObject.GetComponents())
			{
				const UIRect* rect = core::reflection::Cast<const UIRect>(component);
				if (rect != nullptr)
				{
					root = rect;
					break;
				}
			}
			cur = cur->gameObject.transform->GetParent();
		}
		// 메모) 함수 내부에서는 rect를 const로 쓰지만, 반환 되는 rect가 자기 자신 일 수도 있다.
		// UIRect는 모두 포인터 객체고, 외부에서는 const없이 쓸 수도 있으므로 const cast로 캐스팅 한다.
		return const_cast<UIRect&>(*root);
	}
	auto UIInputManager::CreateRectNode(const UIRect& rect, RectNode* parent) -> std::unique_ptr<RectNode>
	{
		auto node = std::make_unique<RectNode>();
		node->rect = const_cast<UIRect*>(&rect);
		node->parent = parent;

		std::queue<const Transform*> bfs;
		for (const Transform* child : rect.gameObject.transform->GetChildren())
			bfs.push(child);

		while (!bfs.empty())
		{
			const Transform* cur = bfs.front();
			bfs.pop();

			if (const UIRect* childRect = FindRect(*cur); childRect != nullptr)
			{
				node->childs.push_back(CreateRectNode(*childRect, node.get()));
				continue;
			}
			for (const Transform* child : cur->GetChildren())
				bfs.push(child);
		}
		return node;
	}
	auto UIInputManager::FindRect(const Transform& transform) -> UIRect*
	{
		for (Component* component : transform.gameObject.GetComponents())
		{
			if (UIRect* rect = core::reflection::Cast<UIRect>(component); rect != nullptr)
				return rect;
		}
		return nullptr;
	}
}//namespace