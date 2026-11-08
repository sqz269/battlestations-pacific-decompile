#include "bsp/native_gui_icon_pool_table.hpp"

#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <vector>

namespace {
void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}
}

// One fixture for the changed path: fill the constructor's 32-entry slab
// table, append the 33rd actual slab, then return/trim/destroy everything.
// Raw payloads belong to this fixture; no live GUI or native ABI is asserted.
int main() {
    try {
        bsp::AllocatorListElement* head = nullptr;
        bsp::AllocatorListDomain list(head);
        bsp::NativeGuiWidgetPoolStorage storage{};
        bsp::NativeGuiWidgetPool<bsp::GuiIconPoolTraits> pool(list, storage);
        pool.initialize();
        require(head == &storage.allocator_00 && storage.table_capacity_30 == 32,
            "initial Icon table/list");
        auto* const original_table = storage.slabs_28;
        std::vector<void*> slots;
        constexpr std::uint32_t count = 32u * 64u + 1u;
        slots.reserve(count);
        for (std::uint32_t i = 0; i < count; ++i) {
            auto* const slot = static_cast<std::byte*>(pool.allocate());
            require(slot == storage.slabs_28[i / 64u] + (i % 64u) * 0x13cu,
                "slot identity across table growth");
            const std::uint32_t cookie = i ^ 0x84692315u;
            std::memcpy(slot, &cookie, sizeof(cookie));
            slots.push_back(slot);
        }
        require(storage.slab_count_2c == 33 && storage.table_capacity_30 == 66
                && storage.slabs_28 != original_table && storage.first_free_slab_34 == 32
                && storage.recursion_24 == 0,
            "33-slab growth/count/lock state");
        for (std::uint32_t i = 0; i < count; ++i) {
            std::uint32_t cookie, slab_index;
            std::memcpy(&cookie, slots[i], sizeof(cookie));
            std::memcpy(&slab_index, static_cast<std::byte*>(slots[i]) + 0x138, sizeof(slab_index));
            require(cookie == (i ^ 0x84692315u) && slab_index == i / 64u,
                "retained payload/trailer");
        }
        for (auto it = slots.rbegin(); it != slots.rend(); ++it) pool.free(*it);
        pool.trim();
        require(storage.slab_count_2c == 0 && storage.table_capacity_30 == 66
                && storage.first_free_slab_34 == 0xffffffffu && storage.recursion_24 == 0,
            "empty slab retirement");
        pool.destroy();
        require(head == nullptr, "allocator-list retirement");
        std::puts("PASS: 2049 Icon slots, 33 slabs, capacity 32 -> 66; payloads, trailers, trim, teardown");
        return 0;
    } catch (const std::exception& error) {
        std::fprintf(stderr, "FAIL: %s\n", error.what());
        return 1;
    }
}
