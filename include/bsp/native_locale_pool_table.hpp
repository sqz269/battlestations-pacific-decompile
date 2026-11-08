#pragma once

#include <cstdint>

namespace bsp {
struct NativeLocaleNodeAllocationProgress;

// Optional diagnostics for the enclosing A9F0B0 operation. This copies no
// native table state and supplies no allocation/storage authority. A caller
// can reject a caught reentry after allocator/free return, as before extraction.
struct NativeLocalePoolTableTrace {
    NativeLocaleNodeAllocationProgress* progress = nullptr;
    void* context = nullptr;
    void (*require_entered)(void*) = nullptr;
};

// Borrow the actual three DWORDs (data/count/capacity). For E1768C these are
// pool+28h/+2Ch/+30h. No copied descriptor, allocation or ownership is created.
struct NativeLocalePoolSlabTable {
    void* storage;
};

// Original A9EA60: ECX=table, RET. Source ABI additionally borrows the optional
// enclosing A9F0B0 diagnostics; they supply no storage or native values.
// Publish 2*capacity+2 BEFORE allocating, reload count/data across callbacks,
// free the current old table, then publish the replacement. No rollback.
void grow_native_string_map_pool_table_00a9ea60(NativeLocalePoolSlabTable,
    NativeLocalePoolTableTrace = {});

// Original A9EE40: ECX=table, stack=pointer cell, RET4. Read the cell only at
// the final nonnull destination store, AFTER growth/free callbacks, and then
// increment the current count even when the computed destination is null.
// Both borrowed storage and the pointer cell must survive all callbacks.
void push_native_string_map_pool_slab_00a9ee40(NativeLocalePoolSlabTable,
    const void* actual_pointer_cell, NativeLocalePoolTableTrace = {});
} // namespace bsp
