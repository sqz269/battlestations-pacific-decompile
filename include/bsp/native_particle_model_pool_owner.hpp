#pragma once

#include "bsp/allocator_list.hpp"

namespace bsp {

// Actual0Ch table header (+28 in the pool). Capture and free a nonnull backing
// pointer; leave pointer/count/capacity untouched. Native ECX header, RET.
void free_native_particle_model_pool_table_00af5d00(void* actual_table_header) noexcept;

// Actual38h owner at00F8D2D0, through the application's SAME E188B4 domain.
// Prepend base/profile, initialize real embedded CS, initialize table metadata,
// reserve32 pointer cells. FH3 cleanup order is table -> section -> base links.
// Native ECX actual fresh owner, EAX same owner, RET. No physical allocation.
void* initialize_native_particle_model_pool_00af6860(void* actual_pool,
    AllocatorListDomain& actual_list);

// Native ECX actual38h owner, RET. Free all slabs and the table, drain positive
// signed lock depth, delete CS, publish base profile and unlink the SAME element.
// No model destructors, metadata clears, or physical owner free are performed.
void destroy_native_particle_model_pool_00af5ff0(void* actual_pool,
    AllocatorListDomain& actual_list) noexcept;

// Native D5DA38 virtual0 -> AF6940; ECX actual owner, RET. No internal lock.
// Free entirely empty32-slot slabs, move last table entry into each hole,
// rewrite all32 moved slot IDs, retry that position, then recompute earliest.
void trim_native_particle_model_pool_00af6940(void* actual_pool) noexcept;

// Host setup: bind the real reconstructed trim operation before publishing the
// one actual static owner/list. No pool bytes, list links or CRT callbacks change
// here. Setup once before startup; raw38h storage and domain must remain alive
// through process exit. The C++ pointers are borrowed bindings, not another pool.
void bind_static_native_particle_model_pool_00f8d2d0(void* actual_pool,
    AllocatorListDomain& actual_list);

// Complete CD7830..CD7845: construct bound pool, real std::atexit(CE0B90 source
// binding), return actual registration status without rollback. No local EH.
int initialize_static_native_particle_model_pool_00cd7830();

// Complete CE0B90..CE0B99: select SAME00F8D2D0 owner; tailcall AF5FF0.
void destroy_static_native_particle_model_pool_00ce0b90() noexcept;

// These are new Win32 C++ interfaces. A model binding still must refer to a
// physically constructed2DCh particle model in a raw slot from native_particle_model_pool_allocate.
// No original vtable/exception ABI, repeated startup or arbitrary rebinding claim.
} // namespace bsp
