#pragma once
#include "bsp/native_texture_loading_cache.hpp"
#include "bsp/native_render_context.hpp"
#include <cstdint>

namespace bsp {
using NativeRenderRemapDecrement = long (__stdcall *)(volatile long*);

struct NativeRenderResourcesRemapContext {
    NativeTextureCacheContext& textures;
    // Same canonical domain used by textures. Existing companions dispatch
    // CURRENT 2D/cube/volume (and other admitted owner) terminals; no forced2D.
    NativeRenderActualOwners& owners;
    NativeRenderRemapDecrement const volatile& decrement_iat_00ce2220;
    const volatile std::uint32_t* renderer_profile_00d5f0a8; // through slot64.
};
struct NativeRenderResourcesRemapAcquired {
    enum class Phase { fresh, release, load, complete };
    Phase phase{Phase::fresh};
    std::uint32_t native_site{};
    bool failed{};
    void* captured_old{}; // Diagnostic identity only; can become freed storage.
    void* loaded{};
    NativeTextureCacheAcquired cache;
};

// Four complete77-byte wrappers. Original ECX actual service; stack actual8h
// name header; EAX loader result; RET4. Release captured old first and clear
// only after returned release. Then capture current F8D394/profile/slot64,
// invoke B319B0(name,0), and publish its exact result without another retain.
// No identity shortcut, null-name fallback, rollback, or restoration on failure.
// Each call requires a fresh persistent acquired; retain failed cache frames
// until the existing concrete child obligations have been resolved.
void* set_native_render_remap_texture0_00b0fd70(void* actual_service,
    void* actual_name_header, NativeRenderResourcesRemapContext&,
    NativeRenderResourcesRemapAcquired&);
void* set_native_render_remap_texture1_00b0fdc0(void* actual_service,
    void* actual_name_header, NativeRenderResourcesRemapContext&,
    NativeRenderResourcesRemapAcquired&);
void* set_native_render_remap_texture2_00b0fe10(void* actual_service,
    void* actual_name_header, NativeRenderResourcesRemapContext&,
    NativeRenderResourcesRemapAcquired&);
void* set_native_render_remap_texture3_00b0fe60(void* actual_service,
    void* actual_name_header, NativeRenderResourcesRemapContext&,
    NativeRenderResourcesRemapAcquired&);

// Complete4-byte accessor: ECX owner, EAX current raw owner+14, RET. No retain,
// argument consumption or profile inference. In B0FEB0 these are actual
// post-effect owners; construction and lifetime of those owners remain external.
void* native_post_effect_material_00b4cba0(const void* actual_owner) noexcept;

// Complete255-byte body. ECX actual service, no stack args, RET; no semantic
// EAX. In order65C,658,654,650: read current owner, skip null, read+14 and call
// complete unchecked B189F0(material,2,null). Then capture66C BEFORE one current
// CE2220 epoch and release/clear66C,670,674,678 in order. Later fields are read
// after prior callbacks. Native storage and all nonnull concrete owner/material
// domains must be live. Does not release the post-effect owners or noise+67C.
void reset_native_render_remap_textures_00b0feb0(void* actual_service,
    NativeRenderResourcesRemapContext&);

// Explicit C++ interfaces; original machine-code/FH3/SEH/application/gameplay
// equivalence is unproven. See docs/NATIVE_RENDER_RESOURCES_REMAP_CT.md.
} // namespace bsp
