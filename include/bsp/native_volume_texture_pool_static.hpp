#pragma once

#include "bsp/d3d9_surface_pool.hpp"

namespace bsp {

// Borrow the distinct actual 0108DBA8 pool and its existing shared allocator
// list companion. B3EC60/B3E2B0 use the same 38h pool/38h slot algorithm as
// surfaces, but volume and surface storage must remain distinct and alive
// through their real CRT callbacks. Binding neither initializes nor registers.
void bind_static_native_volume_texture_pool_0108dba8(D3D9SurfacePool&);

// Full CD7BA0..CD7BB5 (22 bytes), cdecl, EAX atexit status, RET. Initialize
// actual volume pool with B3EC60, then register CE0CC0 through real std::atexit.
// A failed registration retains initialization; a throwing constructor does
// not register cleanup. Explicit one-time startup is the caller's contract.
int initialize_static_native_volume_texture_pool_00cd7ba0();

// Full CE0CC0..CE0CC9 (10 bytes), no arguments: select the SAME 0108DBA8
// storage and tail B3E2B0. Slot payloads must already have been destroyed.
void destroy_static_native_volume_texture_pool_00ce0cc0();

} // namespace bsp
