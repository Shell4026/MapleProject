#pragma once
#include "Export.h"

#include <vector>
#include <cstdint>
namespace sh::game
{
	class Inventory
	{
	public:
		struct Slot
		{
			int64_t itemInstanceId = 0;
			int itemId = -1;
			int quantity = 0;
		};
	public:
		SH_USER_API Inventory();
		SH_USER_API Inventory(const Inventory& other) = delete;
		SH_USER_API Inventory(Inventory&& other) noexcept;

		SH_USER_API auto operator=(const Inventory& other) -> Inventory & = delete;
		SH_USER_API auto operator=(Inventory&& other) noexcept -> Inventory&;

		SH_USER_API void SetItem(int itemId, int slotIdx, int quantity);
		SH_USER_API auto AddItem(int itemId, int quantity) -> bool;
		SH_USER_API auto RemoveItem(int itemId, int quantity) -> bool;
		SH_USER_API auto GetItemCount(int itemId) const -> int;
		/// @brief from에서 to로 슬롯에 있는 아이템을 이동한다. 같은 아이템이면 가능한 만큼 스택을 합친다.
		/// @brief 다른 아이템이면 SwapSlot()과 같다.
		/// @return to에 아이템이 있으면 false
		SH_USER_API auto MoveSlot(int fromIdx, int toIdx) -> bool;
		SH_USER_API auto SwapSlot(int aIdx, int bIdx) -> bool;
		/// @brief 제일 첫 빈 슬롯 인덱스를 반환하는 함수
		/// @param offset 해당 인덱스부터 탐색
		/// @return 빈 슬롯이 없으면 -1
		SH_USER_API auto FindEmptySlotIdx(int offset = 0) const -> int;
		/// @brief 해당 아이템이 존재하는 슬롯의 첫 인덱스를 반환하는 함수
		/// @param itemId 아이템 id
		/// @param offset 해당 인덱스부터 탐색
		/// @return 아이템이 없으면 -1
		SH_USER_API auto FindItemSlot(int itemId, int offset = 0) const -> int;

		SH_USER_API auto GetDirtySlots() const -> const std::vector<int>& { return dirtySlots; }
		SH_USER_API void ClearDirtySlots();

		SH_USER_API auto GetSlots() const -> const std::vector<Slot>& { return slots; }
	private:
		auto IsValidIndex(int idx) const -> bool { return idx >= 0 && idx < static_cast<int>(slots.size()); }
		auto IsEmpty(const Slot& s) const -> bool;
		void MarkDirty(int idx);
	private:
		std::vector<Slot> slots;
		std::vector<int> dirtySlots;
		std::vector<std::uint8_t> dirtyMask; // slots.size()와 동일, 0/1
	};
}//namespace