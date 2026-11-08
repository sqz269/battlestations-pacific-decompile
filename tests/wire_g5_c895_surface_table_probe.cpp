#include "bsp/d3d9_surface_pool.hpp"

#include <array>
#include <cstdio>
#include <cstring>

// Standalone Win32 boundary probe: cross the retained table's initial 32-slab
// capacity through the real pool, then return every slot and trim those slabs.
int main() {
    bsp::AllocatorListElement* head = nullptr;
    bsp::AllocatorListDomain list(head);
    bsp::D3D9SurfacePoolStorage storage{};
    bsp::D3D9SurfacePool pool(list, storage);
    pool.initialize_00b3ec60();
    if (storage.table_capacity_30 != 32 || storage.slab_count_2c != 0) return 1;

    std::array<void*, 32 * 33 + 1> slots{};
    for (std::size_t i = 0; i < slots.size(); ++i) {
        slots[i] = pool.allocate_raw_slot_00b3ed40();
        const auto slab_index = static_cast<std::uint32_t>(i / 32);
        std::uint32_t actual_index;
        std::memcpy(&actual_index,
            static_cast<std::byte*>(slots[i]) + pool.slot_slab_index_offset, 4);
        if (actual_index != slab_index) return 2;
        if (slots[i] != storage.slabs_28[slab_index] + (i % 32) * pool.slot_bytes) return 3;
    }
    if (storage.slab_count_2c != 34 || storage.table_capacity_30 != 66) return 4;
    // Check the first table entries again after the growth/free publication.
    for (std::size_t i = 0; i < 32; ++i)
        if (storage.slabs_28[i] != slots[i * 32]) return 5;

    for (auto* slot : slots) pool.return_raw_slot_00b3d860(slot);
    pool.trim_empty_slabs_00b3e390();
    if (storage.slab_count_2c != 0 || storage.first_free_slab_34 != 0xffffffffu) return 6;
    pool.destroy_00b3e2b0();
    if (head != nullptr) return 7;
    std::puts("PASS: surface pool reserve, 32-to-66 table growth, 1057 returns and trim");
    return 0;
}
