#pragma once
#include "bsp/native_material_pools.hpp"

namespace bsp {
struct NativeMaterialEffectCacheContext;
struct NativeMaterialEffectCacheAcquired;

// Same actual renderer/cache/owner domain; the caller retains the acquired
// effect frame through any cold-loader failure. Numeric native profile view.
struct NativeMaterialFactoryRawContext {
    NativeMaterialEffectCacheContext& effects;
    NativeMaterialEffectCacheAcquired& acquired_effect;
    const volatile std::uint32_t* renderer_profile_00d5f0a8;
};

// Original ECX=actual8h effect-name header, EAX=material, RET. Capture the
// current F8D394 renderer once and invoke its CURRENT callable virtual+48
// (ECX=renderer, stack=name, EAX=owned actual effect, RET4). Rebuilt callers
// must bind a relocated table to the real acquisition implementation; numeric
// original code addresses and semantic EffectCache owners are not callable
// bindings. Acquisition must return nonnull actual effect storage with live
// +04 and writable +B4, in the same canonical retained-owner domain below.
// Allocate/construct from the SAME material pool. On constructor failure,
// return only its raw slot; acquisition/allocation/constructor exceptions do
// not gain a protective effect release. Normal null-slot return still drops
// the acquired effect. This C++ interface does not reproduce native SEH/ABI.
NativeMaterialStorage* create_native_material_for_effect_00535320(
    NativeString& actual_effect_name, void* const volatile& actual_renderer_00f8d394,
    NativeMaterialSlotPool&, NativeRenderActualOwners&);

// Actual D5F0A8+48=B318B0 route over the same allocation/constructor/release
// body. Publish a completed material before releasing the captured effect, so
// a throwing terminal cannot hide that native creator. This output starts null
// and adds no retain or cleanup. Constructor failure returns only its raw slot;
// later failures retain completed native effects and may not replay the frame.
// A null effect is a source-domain error; native later null+04 access-fault
// effects/SEH are not modeled and must not become a successful empty material.
NativeMaterialStorage* create_native_material_for_effect_00535320(
    NativeString& actual_effect_name, void* const volatile& actual_renderer_00f8d394,
    NativeMaterialSlotPool&, NativeRenderActualOwners&, NativeMaterialFactoryRawContext&,
    NativeMaterialStorage** completed_before_effect_release = nullptr);

// Original ECX=raw material slot, RET. Supplies canonical F8D3AC to B17A80.
// Also the destination of state0 funclet C6C240; no material destruction.
void return_native_material_raw_slot_00b17d70(void* actual_slot,
    NativeMaterialSlotPool&);

// Bind each already-created companion to the corresponding actual global
// storage before the CRT startup wrapper runs. These pointers add no pool,
// allocator list, initialization guard, or private exit registry. Binding is
// immutable after startup. Companions/storage/shared list must outlive their
// real std::atexit callbacks, with all payload owners dead before shutdown.
void bind_static_native_material_pool_00f8d3ac(NativeMaterialPool&) noexcept;
void bind_static_native_material_parameter_pool_00f8d3e4(NativeMaterialParameterPool&) noexcept;
// Initialize actual storage first, then register the matching callback and
// return CRT atexit's integer result. No rollback on registration failure.
int initialize_static_native_material_pool_00cd78d0();
int initialize_static_native_material_parameter_pool_00cd78f0();
void destroy_static_native_material_pool_00ce0bf0() noexcept;
void destroy_static_native_material_parameter_pool_00ce0c00() noexcept;

} // namespace bsp
