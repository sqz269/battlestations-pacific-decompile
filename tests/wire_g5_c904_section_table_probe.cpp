// One focused regression: Section table relocation must preserve its live slabs.
#include "bsp/native_gui_section_pool_table.hpp"

#include <array>
#include <cstdio>
#include <cstring>
#include <stdexcept>

namespace {
void require(bool value, const char* message) {
    if (!value) throw std::runtime_error(message);
}
}

int main() {
    bsp::AllocatorListElement* head = nullptr;
    bsp::AllocatorListDomain list(head);
    bsp::NativeGuiWidgetPoolStorage storage;
    std::memset(&storage, 0xa5, sizeof storage);
    bsp::NativeGuiWidgetPool<bsp::GuiSectionPoolTraits> pool(list, storage);
    pool.initialize();
    require(head == &storage.allocator_00 && storage.slab_count_2c == 0 &&
        storage.table_capacity_30 == 32 && storage.recursion_24 == 0,
        "Section constructor did not reserve its actual pool table");

    std::array<void*, 9> slots;
    for (auto& slot : slots) slot = pool.allocate();
    require(storage.slab_count_2c == 2, "Section slots did not use the actual slabs");
    const std::array<std::byte*, 2> slabs{storage.slabs_28[0], storage.slabs_28[1]};
    auto& table = bsp::native_gui_pool_slab_table(storage);
    bsp::reserve_native_gui_section_pool_table_00abed30(table, 40);
    require(table.capacity == 40 && table.count == 2 && table.data[0] == slabs[0] &&
        table.data[1] == slabs[1], "Section table relocation lost a live slab");
    auto* const relocated = table.data;
    bsp::reserve_native_gui_section_pool_table_00abed30(table, 39);
    require(table.data == relocated && table.capacity == 40 && table.count == 2,
        "Section reserve changed a table with sufficient capacity");

    for (auto* slot : slots) pool.free(slot);
    pool.trim();
    require(table.count == 0, "Section retirement left live slab entries");
    pool.destroy();
    require(head == nullptr && table.data == relocated && table.capacity == 40 &&
        table.count == 0, "Section table release reset fields or left allocator links");
    std::puts("c904 Section table: actual reserve32, two live slabs retained by reserve40, "
        "no-op reserve39, unchanged release fields and allocator unlink passed");
}
