#pragma once
#include "bsp/native_render_resources_construction.hpp"
#include "bsp/native_render_service_texture_lifetime.hpp"

namespace bsp {
// Borrow the same raw publication, strings, canonical texture/helper registry,
// frame surfaces and cockpit companions as construction. All reached nonnull
// fields must already have their native lifetimes, including +70, which B14A10
// leaves unwritten and B107F0 later produces. No default preimage is invented.
struct NativeRenderResourcesLifetimeContext {
    NativeRenderServiceBaseContext& base;
    NativeStringRawPoolContext& strings;
    NativeRenderServiceTextureLifetimeContext& textures;
    NativeFrameTargetOwnerContext& frame_targets;
    NativeRenderResourcesConstructionAcquired& construction;
    const volatile std::uint32_t* actual_frame_table_00d5e600;
};

// Complete B0F6E0..B0FBF8: ECX service, RET. Call B52270 on current+34,
// capture+50 then one CE2220 epoch, release 42 fields in exact native order,
// including two independent visits to +1D4; clear+1C4 only after all return.
// Each nonnull child decrements actual+04, dispatches current slot0 only at
// zero, then clears its CURRENT parent field. This does not release noise+67C.
void release_native_render_resources_00b0f6e0(void* actual_service,
    NativeRenderResourcesLifetimeContext&);

// Complete B14F60..B15085: ECX service, RET. Restore D5E480, call B0F6E0,
// release+34/+668/+0C under a fresh captured CE2220 epoch, destroy+69C/+68C
// vectors and+684 string, then B0F0C0. Four-state C++ exception projection
// preserves the original remaining-member schedule; no extra child rollback.
void destroy_native_render_resources_00b14f60(void* actual_service,
    NativeRenderResourcesLifetimeContext&);

// Complete B151C0..B151DD: ECX service, DWORD flags, EAX captured service,
// RET4. Free via shared CRT only when flags&1 and destruction returns.
void* delete_native_render_resources_00b151c0(void* actual_service,
    std::uint32_t flags, NativeRenderResourcesLifetimeContext&);

// Explicit source APIs, not native caller/FH3/SEH ABI. Canonical companions
// retain their existing noexcept terminal boundaries. Unsupported current
// profiles/identities fail explicitly; no arbitrary numeric vtable is called.
} // namespace bsp
