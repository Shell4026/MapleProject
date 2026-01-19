#include "Inventory.h"
#include "Item/ItemDB.h"

#include "Core/Logger.h"

#include <cassert>
#include <limits>
#include <utility>
namespace sh::game
{
	Inventory::Inventory()
	{
		const int cap = 32;
		slots.resize(cap);
		dirtyMask.resize(cap, 0);
		for (auto& s : slots)
		{
			s.itemId = -1;
			s.quantity = 0;
		}
	}

	Inventory::Inventory(const Inventory& other) :
		slots(other.slots),
		dirtySlots(other.dirtySlots),
		dirtyMask(other.dirtyMask)
	{
	}

	Inventory::Inventory(Inventory&& other) noexcept :
		slots(std::move(other.slots)),
		dirtySlots(std::move(other.dirtySlots)),
		dirtyMask(std::move(other.dirtyMask))
	{
	}

	SH_USER_API auto Inventory::operator=(const Inventory& other) -> Inventory&
	{
		if (this == &other)
			return *this;

		slots = other.slots;
		dirtySlots = other.dirtySlots;
		dirtyMask = other.dirtyMask;

		return *this;
	}

	SH_USER_API auto Inventory::operator=(Inventory&& other) noexcept -> Inventory&
	{
		if (this == &other)
			return *this;

		slots = std::move(other.slots);
		dirtySlots = std::move(other.dirtySlots);
		dirtyMask = std::move(other.dirtyMask);

		return *this;
	}

	SH_USER_API auto Inventory::Serialize() const -> core::Json
	{
		core::Json json;
		core::Json& slotsJson = json["slots"];

		int slotIdx = 0;
		for (auto& slot : slots)
		{
			core::Json slotJson;
			slotJson["idx"] = slotIdx++;
			slotJson["itemInstanceId"] = slot.itemInstanceId;
			slotJson["itemId"] = slot.itemId;
			slotJson["quantity"] = slot.quantity;

			slotsJson.push_back(std::move(slotJson));
		}
		return json;
	}

	SH_USER_API auto Inventory::SerializeDirtySlots() const -> core::Json
	{
		core::Json json;
		core::Json& slotsJson = json["slots"];

		for (int i = 0; i < dirtySlots.size(); ++i)
		{
			const int slotIdx = dirtySlots[i];
			const Slot& slot = slots[slotIdx];

			core::Json slotJson;
			slotJson["idx"] = slotIdx;
			slotJson["itemInstanceId"] = slot.itemInstanceId;
			slotJson["itemId"] = slot.itemId;
			slotJson["quantity"] = slot.quantity;

			slotsJson.push_back(std::move(slotJson));
		}
		return json;
	}

	SH_USER_API void Inventory::Deserialize(const core::Json& json)
	{
		auto it = json.find("slots");
		if (it == json.end())
			return;
		const core::Json& slotsJson = it.value();

		for (auto& slotJson : slotsJson)
		{
			const int slotIdx = slotJson.value("idx", 0);
			auto& slot = slots[slotIdx];
			slot.itemInstanceId = slotJson.value("itemInstanceId", 0);
			slot.itemId = slotJson.value("itemId", -1);
			slot.quantity = slotJson.value("quantity", 0);
		}
	}

	void Inventory::SetItem(int itemId, int slotIdx, int quantity)
	{
		assert(IsValidIndex(slotIdx));
		assert(quantity >= 0);

		Slot& slot = slots[slotIdx];
		slot.itemId = itemId;
		slot.quantity = quantity;
		MarkDirty(slotIdx);
	}
	SH_USER_API auto Inventory::AddItem(int itemId, int quantity) -> bool
	{
		if (quantity <= 0)
			return true;

		const ItemInfo* info = ItemDB::GetInstance()->GetItemInfo(itemId);
		if (info == nullptr)
		{
			SH_ERROR_FORMAT("Not found the item info: {}", itemId);
			return false;
		}

		const int maxStack = std::max(1, info->maxStack);

		// 들어갈 수 있는지 계산
		int64_t capacity = 0;
		for (const Slot& s : slots)
		{
			if (!IsEmpty(s) && s.itemId == itemId)
			{
				if (s.quantity < maxStack)
					capacity += (maxStack - s.quantity);
			}
		}
		int emptyCount = 0;
		for (const Slot& s : slots)
		{
			if (IsEmpty(s))
				++emptyCount;
		}
		capacity += static_cast<int64_t>(emptyCount) * maxStack;

		if (capacity < quantity)
			return false;

		// 기존 스택 -> 빈 슬롯 순으로 채우기
		int remaining = quantity;

		for (int i = 0; i < slots.size() && remaining > 0; ++i)
		{
			Slot& s = slots[i];
			if (IsEmpty(s) || s.itemId != itemId)
				continue;

			if (s.quantity >= maxStack)
				continue;

			const int canAdd = std::min(maxStack - s.quantity, remaining);
			s.quantity += canAdd;
			remaining -= canAdd;
			MarkDirty(i);
		}
		for (int i = 0; i < static_cast<int>(slots.size()) && remaining > 0; ++i)
		{
			Slot& s = slots[i];
			if (!IsEmpty(s))
				continue;

			const int put = std::min(maxStack, remaining);
			s.itemId = itemId;
			s.quantity = put;
			remaining -= put;
			MarkDirty(i);
		}

		assert(remaining == 0);
		return true;
	}
	SH_USER_API auto Inventory::RemoveItem(int itemId, int quantity) -> bool
	{
		if (quantity <= 0)
			return true;

		const int total = GetItemCount(itemId);
		if (total < quantity)
			return false;

		int remaining = quantity;
		for (int i = slots.size() - 1; i >= 0 && remaining > 0; --i)
		{
			Slot& s = slots[i];
			if (IsEmpty(s) || s.itemId != itemId)
				continue;

			const int take = std::min(s.quantity, remaining);
			s.quantity -= take;
			remaining -= take;

			if (s.quantity == 0)
				s.itemId = -1;

			MarkDirty(i);
		}

		assert(remaining == 0);
		return true;
	}
	SH_USER_API auto Inventory::GetItemCount(int itemId) const -> int
	{
		int64_t sum = 0;
		for (const Slot& slot : slots)
		{
			if (!IsEmpty(slot) && slot.itemId == itemId)
				sum += slot.quantity;
		}
		if (sum > std::numeric_limits<int>::max())
			return std::numeric_limits<int>::max();
		return sum;
	}
	SH_USER_API auto Inventory::MoveSlot(int fromIdx, int toIdx) -> bool
	{
		if (!IsValidIndex(fromIdx) || !IsValidIndex(toIdx) || fromIdx == toIdx)
			return false;

		Slot& from = slots[fromIdx];
		Slot& to = slots[toIdx];

		if (IsEmpty(from))
			return false;

		if (IsEmpty(to))
		{
			to = from;
			from.itemId = -1;
			from.quantity = 0;
			MarkDirty(fromIdx);
			MarkDirty(toIdx);
			return true;
		}

		// 스택 합치기
		if (to.itemId == from.itemId)
		{
			const ItemInfo* info = ItemDB::GetInstance()->GetItemInfo(from.itemId);
			if (info == nullptr)
			{
				SH_ERROR_FORMAT("Not found the item info: {}", from.itemId);
				return false;
			}
			const int maxStack = std::max(1, info->maxStack);

			if (to.quantity >= maxStack)
				return false;

			const int canMove = std::min(maxStack - to.quantity, from.quantity);
			to.quantity += canMove;
			from.quantity -= canMove;

			if (from.quantity == 0)
				from.itemId = -1;

			MarkDirty(fromIdx);
			MarkDirty(toIdx);
			return true;
		}

		return SwapSlot(fromIdx, toIdx);
	}
	SH_USER_API auto Inventory::SwapSlot(int aIdx, int bIdx) -> bool
	{
		if (!IsValidIndex(aIdx) || !IsValidIndex(bIdx) || aIdx == bIdx)
			return false;

		std::swap(slots[aIdx], slots[bIdx]);
		MarkDirty(aIdx);
		MarkDirty(bIdx);
		return true;
	}
	SH_USER_API auto Inventory::FindEmptySlotIdx(int offset) const -> int
	{
		if (offset < 0) 
			offset = 0;
		for (int i = offset; i < slots.size(); ++i)
		{
			if (IsEmpty(slots[i]))
				return i;
		}
		return -1;
	}
	SH_USER_API auto Inventory::FindItemSlot(int itemId, int offset) const -> int
	{
		if (offset < 0) 
			offset = 0;
		for (int i = offset; i < slots.size(); ++i)
		{
			const Slot& slot = slots[i];
			if (!IsEmpty(slot) && slot.itemId == itemId)
				return i;
		}
		return -1;
	}
	SH_USER_API void Inventory::ClearDirtySlots()
	{
		for (int idx : dirtySlots)
		{
			if (IsValidIndex(idx))
				dirtyMask[idx] = 0;
		}
		dirtySlots.clear();
	}
	auto Inventory::IsEmpty(const Slot& s) const -> bool
	{
		return s.itemId == -1 || s.quantity == 0;
	}
	void Inventory::MarkDirty(int idx)
	{
		assert(IsValidIndex(idx));
		if (dirtyMask[idx] == 0)
		{
			dirtyMask[idx] = 1;
			dirtySlots.push_back(idx);
		}
	}
}//namespace