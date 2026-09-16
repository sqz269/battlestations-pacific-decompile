#pragma once

#include "bsp/native_weak_owner.hpp"

namespace bsp {

// Bind the companion that already targets the application's actual38h F8D344
// particle-parameter pool storage and shared E188B4 allocator-list domain.
// This adds no owner, storage, list head, slab table, or CRT callback.
void bind_static_native_particle_parameter_pool_00f8d344(
    NativeWeakHandlePool& actual_pool);

// Complete CD78B0..CD78C5: construct the bound F8D344 pool first, then
// register CE0BE0 through atexit and return that exact result. Registration
// failure does not roll construction back; repeated startup is unsupported.
int initialize_static_native_particle_parameter_pool_00cd78b0(
    NativeWeakPoolAtexit register_atexit = &std::atexit);

// Complete CE0BE0..CE0BE9: select the SAME bound F8D344 pool and destroy it
// through the existing generic B002C0 implementation.
void destroy_static_native_particle_parameter_pool_00ce0be0();

// These are new Win32 C++ interfaces. Application storage/bootstrap binding,
// original CRT ABI execution and gameplay validation remain separate.

} // namespace bsp
