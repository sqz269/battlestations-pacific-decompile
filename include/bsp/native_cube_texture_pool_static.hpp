#pragma once

#include "bsp/allocator_list.hpp"

namespace bsp {

// Explicit host setup for the one actual canonical 38h global pool and shared
// allocator-list domain. Establish the genuine D61944/B3E690 trim binding before
// publishing these borrowed host pointers. This does not initialize the pool,
// write its bytes, reset the shared list, or register an exit callback.
// Bind once before startup; the same pool/list and binding remain alive through
// the process exit callback. Storage is aligned and initially unconstructed.
void bind_static_native_cube_texture_pool_0108db70(
    void* actual_pool, AllocatorListDomain& actual_list);

// Complete CD7B80..CD7B96: no arguments, EAX actual _atexit status, RET.
// Initialize the bound actual pool, then call real std::atexit with CE0CB0's
// source shutdown binding. Return registration status without rollback.
// Construction failure propagates before registration; there is no own EH.
int initialize_static_native_cube_texture_pool_00cd7b80();

// Complete CE0CB0..CE0CBA: no arguments; native tailcall B3E5B0 with actual
// global pool ECX. Destroy that same pool with the same shared list domain.
void destroy_static_native_cube_texture_pool_00ce0cb0() noexcept;

// New MSVC Win32 C++ interfaces; CRT dispatcher/callback storage, original ABI,
// arbitrary rebinding, repeated startup/shutdown, and gameplay are not proved.
} // namespace bsp
