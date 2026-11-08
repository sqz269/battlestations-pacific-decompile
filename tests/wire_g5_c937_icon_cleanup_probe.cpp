#include "bsp/native_gui_icon_pool_table.hpp"

#include <cstdio>
#include <cstring>
#include <stdexcept>

namespace {
void require(bool value, const char* message) {
    if (!value) throw std::runtime_error(message);
}
}

// One teardown case with two actual slabs: the ordinary destructor must free
// them and their table once, preserve the stale table header, and unlink.
// Source behavior only; no original ABI or live game execution is claimed.
int main() {
    try {
        bsp::NativeGuiPoolSlabTable empty{};
        bsp::free_native_gui_icon_pool_table_00ab14f0(&empty);
        require(!empty.data && !empty.count && !empty.capacity, "null table unchanged");

        bsp::AllocatorListElement* head = nullptr;
        bsp::AllocatorListDomain list(head);
        bsp::NativeGuiWidgetPoolStorage storage{};
        bsp::NativeGuiWidgetPool<bsp::GuiIconPoolTraits> pool(list, storage);
        pool.initialize();
        for (unsigned i = 0; i != 65; ++i)
            require(pool.allocate() != nullptr, "Icon slot allocation");
        require(storage.slab_count_2c == 2 && storage.table_capacity_30 == 32
            && storage.recursion_24 == 0 && head == &storage.allocator_00,
            "live two-slab Icon pool");
        unsigned char before[sizeof(bsp::NativeGuiPoolSlabTable)];
        std::memcpy(before, &storage.slabs_28, sizeof(before));
        pool.destroy();
        require(std::memcmp(before, &storage.slabs_28, sizeof(before)) == 0,
            "destructor preserves complete stale table header");
        require(head == nullptr && storage.recursion_24 == 0,
            "Icon critical-section/list teardown");
        std::puts("PASS: Icon 65 slots, two slabs; null release, retained table header, list teardown");
        return 0;
    } catch (const std::exception& error) {
        std::fprintf(stderr, "FAIL: %s\n", error.what());
        return 1;
    }
}
