#include "bsp/native_render_pointer_arrays.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstdio>
#include <cstdlib>

// Standalone Win32 regression for B1CBE0's source-cell timing and null branch.
// Link the production pointer-array translation unit with these heap imports.
// No game is launched and this is not a native differential comparison.
namespace bsp {
void* singleton_lifetime_allocate(const SingletonAllocationRequest& request) {
    if (request.native_bytes != request.host_bytes) std::abort();
    return std::malloc(request.host_bytes);
}
void singleton_lifetime_free(void* memory) noexcept { std::free(memory); }
}

int main() {
    bsp::NativeRenderPointerArrayStorage array{};
    // The source aliases the data word: growth must happen before its load.
    bsp::append_native_render_group_pointer_00b1cbe0(array, &array.data_00);
    if (!array.data_00 || array.count_04 != 1 || array.capacity_08 != 1 ||
        array.data_00[0] != array.data_00) return 1;
    auto** const first = array.data_00;
    int actual_entry;
    void* entry = &actual_entry;
    bsp::append_native_instance_entry_pointer_00b1cb80(array, &entry);
    if (array.count_04 != 2 || array.capacity_08 != 2 ||
        array.data_00[0] != first || array.data_00[1] != entry) return 2;
    bsp::singleton_lifetime_free(array.data_00);

    // Native TEST/JZ bypasses an unreadable source when data+count*4 is null.
    bsp::NativeRenderPointerArrayStorage null_destination{nullptr, 0, 1};
    bsp::append_native_render_group_pointer_00b1cbe0(null_destination, nullptr);
    if (null_destination.data_00 || null_destination.count_04 != 1 ||
        null_destination.capacity_08 != 1) return 3;
    std::puts("group/entry append growth, source alias, and null destination passed");
}
