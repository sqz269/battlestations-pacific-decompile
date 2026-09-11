#pragma once

#include "bsp/allocator_list.hpp"

namespace bsp {

// B60350: native ECX actual38h pool, RET; D62AF0 virtual slot0.
// Free each slab whose WORD+940 is32, compact the current pointer table,
// rewrite all32 moved slot tokens, then recompute first slab with free slots.
// No lock acquisition, capacity shrink, payload destruction or table clearing.
// Borrow valid backed pool/table/slabs from the shared native allocation domain.
void trim_native_hardware_layout_pool_00b60350(void* actual_pool) noexcept;

// Explicit host setup: bind the real raw first12-byte allocator-list element
// to the complete native profile before the pool initializer publishes it.
// This does not write the raw pool or add it to the shared E188B4 list.
void bind_native_hardware_layout_pool_trim_00d62af0(
    void* actual_pool, AllocatorListDomain& actual_list);

// New MSVC Win32 interfaces; native vtable words remain raw data. No substitute
// pool, placeholder virtual target, native calling-convention or game claim.
} // namespace bsp
