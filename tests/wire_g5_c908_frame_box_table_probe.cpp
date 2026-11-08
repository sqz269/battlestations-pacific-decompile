#include "bsp/native_gui_frame_box_pool_table.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

// One integration boundary: the production FrameBox owner reserves 32 slab
// pointers, then its 513th slot grows that same header to 66 without losing
// payloads or the trailing slab indices. No alternate allocator algorithm.
int main() {
    bsp::AllocatorListElement* head = nullptr;
    bsp::AllocatorListDomain list(head);
    bsp::NativeGuiWidgetPoolStorage storage{};
    bsp::NativeGuiWidgetPool<bsp::GuiFrameBoxPoolTraits> pool(list, storage);
    auto check = [](bool condition) { if (!condition) std::abort(); };
    pool.initialize();
    check(storage.table_capacity_30 == 32 && storage.slab_count_2c == 0);
    auto* initial = storage.slabs_28;
    bsp::native_gui_frame_box_pool_table_reserve_00acf860(
        bsp::native_gui_pool_slab_table(storage), 32);
    check(storage.slabs_28 == initial);
    std::vector<void*> slots;
    for (std::uint32_t i = 0; i < 513; ++i) {
        void* slot = pool.allocate();
        std::memcpy(slot, &i, 4);
        slots.push_back(slot);
    }
    check(storage.slab_count_2c == 33 && storage.table_capacity_30 == 66);
    check(storage.first_free_slab_34 == 32 && storage.recursion_24 == 0);
    for (std::uint32_t i = 0; i < slots.size(); ++i) {
        std::uint32_t value, index;
        std::memcpy(&value, slots[i], 4);
        std::memcpy(&index, static_cast<unsigned char*>(slots[i]) + 0x11c, 4);
        check(value == i && index == i / 16);
        check(slots[i] == storage.slabs_28[index] + (i % 16) * 0x120);
    }
    pool.destroy();
    check(head == nullptr);
    std::puts("FrameBox table boundary passed: 513 slots, capacity32->66");
}
