#pragma once

#include "bsp/allocator_list.hpp"

namespace bsp {

// Complete B3E690..B3E72F: native ECX actual38h pool, RET. New MSVC Win32
// C++ interface over actual borrowed pool/slab/table storage. Requires valid
// reached extents; neither enters nor leaves the pool's critical section.
// Frees empty6C4h slabs, compacts the table and rewrites moved slot index words.
void trim_native_cube_texture_pool_00b3e690(void* actual_pool) noexcept;

// Host dispatch metadata only: bind the actual first12-byte allocator element
// to genuine D61944/B3E690 trim in the same shared list domain. Establish this
// before native initialization publishes the node: real new-handler processing
// can traverse the list. This neither initializes nor publishes the raw pool.
void bind_native_cube_texture_pool_trim_00d61944(
    void* actual_pool, AllocatorListDomain& actual_list);

} // namespace bsp
