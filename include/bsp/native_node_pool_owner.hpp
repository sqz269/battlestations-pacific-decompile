#pragma once

#include "bsp/allocator_list.hpp"

namespace bsp {

// Actual0Ch table header (+28 in the pool). Capture and free a nonnull backing
// pointer; leave pointer/count/capacity untouched. Native ECX header, RET.
void free_native_node_pool_table_00b6ddf0(void* actual_table_header) noexcept;

// Actual38h owner at0108FF58, through the application's SAME E188B4 domain.
// Prepend base/profile, initialize real embedded CS, initialize table metadata,
// reserve32 pointer cells. FH3 cleanup order is table -> section -> base links.
// Native ECX actual fresh owner, EAX same owner, RET. No physical allocation.
void* initialize_native_node_pool_00b6e980(void* actual_pool,
    AllocatorListDomain& actual_list);

// Native ECX actual38h owner, RET. Free all slabs and the table, drain positive
// signed lock depth, delete CS, publish base profile and unlink the SAME element.
// No node destructors, metadata clears, or physical owner free are performed.
void destroy_native_node_pool_00b6e3d0(void* actual_pool,
    AllocatorListDomain& actual_list) noexcept;

// Native D62C78 virtual0 -> B6EA60; ECX actual owner, RET. No internal lock.
// Free entirely empty32-slot slabs, move last table entry into each hole,
// rewrite all32 moved slot IDs, retry that position, then recompute earliest.
void trim_native_node_pool_00b6ea60(void* actual_pool) noexcept;

// Host setup: bind the real reconstructed trim operation before publishing the
// one actual static owner/list. No pool bytes, list links or CRT callbacks change
// here. Setup once before startup; raw38h storage and domain must remain alive
// through process exit. The C++ pointers are borrowed bindings, not another pool.
void bind_static_native_node_pool_0108ff58(void* actual_pool,
    AllocatorListDomain& actual_list);

// Complete CD7D10..CD7D25: construct bound pool, real std::atexit(CE0E20 source
// binding), return actual registration status without rollback. No local EH.
int initialize_static_native_node_pool_00cd7d10();

// Complete CE0E20..CE0E29: select SAME0108FF58 owner; tailcall B6E3D0.
void destroy_static_native_node_pool_00ce0e20() noexcept;

// These are new Win32 C++ interfaces. NativeNodeBinding still must refer to a
// physically constructed174h node in a raw slot from native_node_pool_allocation.
// No original vtable/exception ABI, repeated startup or arbitrary rebinding claim.
} // namespace bsp
