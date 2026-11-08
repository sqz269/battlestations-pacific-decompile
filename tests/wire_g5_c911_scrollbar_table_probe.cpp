#include "bsp/native_gui_scrollbar_pool_table.hpp"

#include <array>
#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <unordered_set>

namespace {
void check(bool value, const char* message) {
    if (!value) throw std::runtime_error(message);
}
std::uint32_t word(const void* source) {
    std::uint32_t result;
    std::memcpy(&result, source, sizeof result);
    return result;
}
}

int main() {
    try {
        using Traits = bsp::GuiScrollbarPoolTraits;
        bsp::AllocatorListElement* head = nullptr;
        bsp::AllocatorListDomain list(head);
        bsp::NativeGuiWidgetPoolStorage storage{};
        bsp::NativeGuiWidgetPool<Traits> pool(list, storage);
        pool.initialize();
        check(head == &storage.allocator_00 && storage.table_capacity_30 == 32
            && storage.slab_count_2c == 0, "Scrollbar constructor did not reserve its actual table");

        // One scenario crosses the 32-slab table boundary, then trims an early
        // slab while later live slots must retain payloads and updated trailers.
        std::array<void*, 529> slots{};
        std::array<std::byte*, 32> first_slabs{};
        std::unordered_set<void*> unique;
        for (std::size_t i = 0; i < slots.size(); ++i) {
            slots[i] = pool.allocate();
            check(slots[i] && unique.insert(slots[i]).second, "duplicate or null live slot");
            const auto payload = static_cast<std::uint32_t>(i + 1);
            std::memcpy(slots[i], &payload, sizeof payload);
            if (i == 511) std::memcpy(first_slabs.data(), storage.slabs_28, sizeof first_slabs);
        }
        check(storage.slab_count_2c == 34 && storage.table_capacity_30 == 66
            && storage.recursion_24 == 0, "table growth or lock depth changed");
        check(std::memcmp(first_slabs.data(), storage.slabs_28, sizeof first_slabs) == 0,
            "growing the actual table lost a retained slab");
        for (std::size_t i = 0; i < Traits::slots_per_slab; ++i) pool.free(slots[i]);
        pool.trim();
        check(storage.slab_count_2c == 33, "empty first slab was not trimmed");
        for (std::size_t i = Traits::slots_per_slab; i < slots.size(); ++i) {
            check(word(slots[i]) == i + 1, "live slot payload changed across growth/trim");
            const auto id = word(static_cast<std::byte*>(slots[i]) + Traits::slot_slab_index_offset);
            check(id < storage.slab_count_2c, "moved slab trailer is stale");
            const auto slot_address = reinterpret_cast<std::uintptr_t>(slots[i]);
            const auto slab_address = reinterpret_cast<std::uintptr_t>(storage.slabs_28[id]);
            check(slot_address >= slab_address
                && slot_address < slab_address + Traits::slots_per_slab * Traits::slot_bytes,
                "live slot no longer belongs to its table entry");
            pool.free(slots[i]);
        }
        pool.trim();
        check(storage.slab_count_2c == 0 && storage.table_capacity_30 == 66,
            "final trim changed retained table ownership");
        pool.destroy();
        // Native destruction frees the table without clearing its stale data
        // word; only the allocator-list owner must have been unlinked here.
        check(head == nullptr, "pool destruction left its allocator-list owner linked");
        std::puts("Scrollbar actual table: 529 unique slots, capacity32->66, live-slab trim and teardown passed");
        return 0;
    } catch (const std::exception& error) {
        std::fprintf(stderr, "Scrollbar table probe: %s\n", error.what());
        return 1;
    }
}
