#include "bsp/native_gui_grid_pool_table.hpp"

#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <vector>

namespace {
void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}
}

// One relocation boundary: exhaust the 32 reserved sixteen-slot Grid slabs,
// grow their actual table, then retire those same slots. Source fixture only.
int main() {
    try {
        bsp::AllocatorListElement* head = nullptr;
        bsp::AllocatorListDomain list(head);
        bsp::NativeGuiWidgetPoolStorage storage{};
        bsp::NativeGuiWidgetPool<bsp::GuiGridPoolTraits> pool(list, storage);
        pool.initialize();
        require(head == &storage.allocator_00 && storage.table_capacity_30 == 32,
            "Grid initial table/list");
        auto* const initial_table = storage.slabs_28;
        std::vector<void*> slots;
        constexpr std::uint32_t count = 32u * 16u + 1u;
        for (std::uint32_t i = 0; i < count; ++i) {
            auto* const slot = static_cast<std::byte*>(pool.allocate());
            require(slot == storage.slabs_28[i / 16u] + (i % 16u) * 0x128u,
                "Grid slot identity");
            std::memcpy(slot, &i, sizeof(i));
            slots.push_back(slot);
        }
        require(storage.slab_count_2c == 33 && storage.table_capacity_30 == 66
                && storage.slabs_28 != initial_table && storage.first_free_slab_34 == 32
                && storage.recursion_24 == 0, "Grid growth/count/lock");
        for (std::uint32_t i = 0; i < count; ++i) {
            std::uint32_t payload, slab_index;
            std::memcpy(&payload, slots[i], sizeof(payload));
            std::memcpy(&slab_index, static_cast<std::byte*>(slots[i]) + 0x124u,
                sizeof(slab_index));
            require(payload == i && slab_index == i / 16u,
                "Grid retained payload/trailer");
        }
        for (auto it = slots.rbegin(); it != slots.rend(); ++it) pool.free(*it);
        pool.trim();
        require(storage.slab_count_2c == 0 && storage.table_capacity_30 == 66
                && storage.first_free_slab_34 == 0xffffffffu && storage.recursion_24 == 0,
            "Grid trim");
        pool.destroy();
        require(head == nullptr, "Grid allocator-list retirement");
        std::puts("PASS: 513 Grid slots; 33 slabs; table 32 -> 66; payloads, trailers, trim, teardown");
        return 0;
    } catch (const std::exception& error) {
        std::fprintf(stderr, "FAIL: %s\n", error.what());
        return 1;
    }
}
