#pragma once

#include "bsp/native_directional_shadow_construction_block.hpp"
#include "bsp/native_directional_shadow_owner_lifetime.hpp"

namespace bsp {
struct NativeVfsNameResolutionContext;

// Borrowed actual application domains; no allocator, terminal stub or renderer
// callback is introduced. These bindings and the block outlive retained native
// children/cache operations, including survivors of a failed constructor.
struct NativeDirectionalShadowConstructionContext {
    NativeCameraEnvironment& cameras;
    const NativeNodeRawConstants& node_constants;
    NativeViewportRegistry& viewports;
    NativeTextureCacheContext& texture_cache;
    NativeVfsNameResolutionContext& texture_names;
    NativeDirectionalShadowOwnerContext& lifetime;
    // Same publication cell as texture_cache.textures.current_renderer_00f8d394.
    // Read at each reached native site; never snapshot these in preparation.
    const void* volatile& renderer_00f8d394;
    const void* const volatile& target_00f8bbf0;
    const volatile std::uint32_t* const renderer_table_00d5f0a8;
    const char* const white_name_00ce77a4;
    const char* const camera_names_00d5b5bc_00d5b5a8_00d5b594_00d5b580[4];
    // The actual cache string bridge must use the SAME three raw pool cells as
    // cameras.nodes.require_raw_name_pool(). Its private fields cannot be
    // compared here; this is a caller wiring obligation. The existing VFS
    // binder verifies the exposed texture/VFS pool and manager identities.
};

// Complete normal bodies: A8E2E0[1408], A8FA30[641], A8FD30[96]. Original
// constructors: ECX actual508h, stacked light, EAX same storage, RET4; factory:
// ECX light, EAX allocated owner/null, RET. These context/admission APIs are new
// C++ interfaces, not binary ABI replacements or executable private FH3 frames.
// Direct entries require aligned writable unused508h storage and never free it.
// Only the factory owns shared-CRT508h allocation and its raw-free unwind.
// Exactly one prepared admission is consumed before native effects. Nested
// bodies share it; settled storage and native survivors require explicit later
// retirement/quiescence. No implicit cleanup of published children is added.
void* construct_native_directional_shadow_base_00a8e2e0(void* actual_508h,
    void* actual_light, NativeDirectionalShadowConstructionContext&,
    NativeDirectionalShadowConstructionBlock::Admission&&);
void* construct_native_directional_shadow_owner_00a8fa30(void* actual_508h,
    void* actual_light, NativeDirectionalShadowConstructionContext&,
    NativeDirectionalShadowConstructionBlock::Admission&&);
void* allocate_native_directional_shadow_owner_00a8fd30(void* actual_light,
    NativeDirectionalShadowConstructionContext&,
    NativeDirectionalShadowConstructionBlock::Admission&&);
} // namespace bsp
