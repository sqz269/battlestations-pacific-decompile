#include "bsp/gui_text_factory.hpp"

#include <array>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <stdexcept>

namespace {
void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}
}

// One regression case: cross the actual pool's initial 32-slab table boundary.
// This reaches reserve -> capacity and append -> grow -> capacity through the
// same retained +28h header, then uses every old slot before native teardown.
int main() {
    bsp::AllocatorListElement* head = nullptr;
    bsp::AllocatorListDomain domain(head);
    bsp::NativeGuiTextPoolStorage storage{};
    bsp::NativeGuiTextPool pool(domain, storage);
    pool.initialize_00ab77c0();
    require(head == &storage.allocator_00 && storage.table_capacity_30 == 32,
        "Text pool startup did not retain its 32-slot table");
    auto* const initial_table = storage.slabs_28;
    bsp::reserve_gui_text_pool_table_00ab7690(&storage.slabs_28, 16);
    require(storage.slabs_28 == initial_table && storage.table_capacity_30 == 32,
        "Text table reserve shrank/replaced existing storage");

    std::array<void*, 32 * 64 + 1> slots{};
    for (std::size_t i = 0; i < slots.size(); ++i) {
        slots[i] = pool.allocate_raw_slot_00ab78a0();
        const auto marker = static_cast<std::uint32_t>(i + 1);
        std::memcpy(slots[i], &marker, sizeof marker);
    }
    require(storage.slab_count_2c == 33 && storage.table_capacity_30 == 66 &&
        storage.slabs_28 != initial_table && storage.recursion_24 == 0,
        "Text table growth did not preserve count/capacity/lock state");
    for (std::size_t i = 0; i < slots.size(); ++i) {
        std::uint32_t marker;
        std::memcpy(&marker, slots[i], sizeof marker);
        require(marker == i + 1, "Text table growth changed an existing slot");
        pool.return_raw_slot_00ab75a0(slots[i]);
    }
    pool.trim_empty_slabs_00ab7500();
    require(storage.slab_count_2c == 0 && storage.first_free_slab_34 == 0xffffffffu,
        "Text pool could not retire the grown table's slabs");
    pool.destroy_00ab7420();
    require(head == nullptr, "Text pool teardown retained allocator-list membership");
    std::puts("c870 Text pool: 2049 slots, 32 -> 66 table capacity, return/trim/destroy PASS");
}
