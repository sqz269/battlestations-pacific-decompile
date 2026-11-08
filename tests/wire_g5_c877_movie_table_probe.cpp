#include "bsp/native_gui_movie_pool_table.hpp"

#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <vector>

namespace {
void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}
}

// One boundary fixture: exhaust the constructor's 32 Movie slabs (four
// slots each), force a table relocation, then retire the same payloads.
// This verifies source behavior only; it is not a live GUI or original-ABI test.
int main() {
    try {
        bsp::AllocatorListElement* head = nullptr;
        bsp::AllocatorListDomain list(head);
        bsp::NativeGuiWidgetPoolStorage storage{};
        bsp::NativeGuiWidgetPool<bsp::GuiMoviePoolTraits> pool(list, storage);
        pool.initialize();
        require(head == &storage.allocator_00 && storage.table_capacity_30 == 32,
            "Movie initial table/list");
        auto* const old_table = storage.slabs_28;
        std::vector<void*> slots;
        constexpr std::uint32_t count = 32u * 4u + 1u;
        for (std::uint32_t i = 0; i < count; ++i) {
            auto* const slot = static_cast<std::byte*>(pool.allocate());
            require(slot == storage.slabs_28[i / 4u] + (i % 4u) * 0x134u,
                "Movie slot identity");
            const std::uint32_t cookie = i ^ 0x8775u;
            std::memcpy(slot, &cookie, sizeof(cookie));
            slots.push_back(slot);
        }
        require(storage.slab_count_2c == 33 && storage.table_capacity_30 == 66
                && storage.slabs_28 != old_table && storage.first_free_slab_34 == 32
                && storage.recursion_24 == 0,
            "Movie growth/count/lock");
        for (std::uint32_t i = 0; i < count; ++i) {
            std::uint32_t cookie, slab_index;
            std::memcpy(&cookie, slots[i], sizeof(cookie));
            std::memcpy(&slab_index, static_cast<std::byte*>(slots[i]) + 0x130u, sizeof(slab_index));
            require(cookie == (i ^ 0x8775u) && slab_index == i / 4u,
                "Movie retained payload/trailer");
        }
        for (auto it = slots.rbegin(); it != slots.rend(); ++it) pool.free(*it);
        pool.trim();
        require(storage.slab_count_2c == 0 && storage.table_capacity_30 == 66
                && storage.first_free_slab_34 == 0xffffffffu && storage.recursion_24 == 0,
            "Movie trim");
        pool.destroy();
        require(head == nullptr, "Movie list retirement");
        std::puts("PASS: 129 Movie slots; 33 slabs; table 32 -> 66; payloads, trailers, trim, teardown");
        return 0;
    } catch (const std::exception& error) {
        std::fprintf(stderr, "FAIL: %s\n", error.what());
        return 1;
    }
}
