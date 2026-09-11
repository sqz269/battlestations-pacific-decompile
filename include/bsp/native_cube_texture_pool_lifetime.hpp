#pragma once

#include "bsp/allocator_list.hpp"

namespace bsp {

// Complete B3F090: ECX actual writable 38h pool, EAX same pool, RET.
// Storage contains the native 0Ch list element and actual Win32 section at +0C.
// Establish the genuine D61944/B3E690 trim binding in this same shared list
// domain before calling: real new-handler processing can reach the published
// node. Call once for uninitialized storage; preserve the existing shared list.
void* initialize_native_cube_texture_pool_00b3f090(
    void* pool, AllocatorListDomain& list);

// Complete B3E5B0: ECX actual initialized pool, RET. Frees current slabs and
// table, drains positive recursion, deletes the section, and unlinks the node.
// It neither destroys logical slot owners nor clears retained native fields.
// Actual pool storage and its host virtual binding remain caller-owned.
void destroy_native_cube_texture_pool_00b3e5b0(
    void* pool, AllocatorListDomain& list) noexcept;

// Complete B3D3F0: ECX actual table header (pool+28), RET. Frees its current
// nonnull first DWORD and clears nothing. The argument is not a pool pointer.
void free_native_cube_texture_pool_table_00b3d3f0(void* table_header) noexcept;

} // namespace bsp
