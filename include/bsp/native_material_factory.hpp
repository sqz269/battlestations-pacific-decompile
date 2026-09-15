#pragma once
#include "bsp/native_material_pools.hpp"
#include <memory>

namespace bsp {
struct NativeMaterialEffectCacheContext;
struct NativeMaterialEffectCacheAcquired;
struct NativeMaterialFactoryAcquired {
    enum class Phase { empty, effect, allocation, construction, effect_release, complete };
    NativeMaterialFactoryAcquired();
    ~NativeMaterialFactoryAcquired();
    NativeMaterialFactoryAcquired(const NativeMaterialFactoryAcquired&) = delete;
    NativeMaterialFactoryAcquired& operator=(const NativeMaterialFactoryAcquired&) = delete;
    Phase phase{Phase::empty};
    std::uint32_t native_site{};
    std::unique_ptr<NativeMaterialEffectCacheAcquired> cache;
    void* effect{};
    void* raw_slot{};
    NativeMaterialStorage* material{};
    RenderCommandReference* companion{};
    void* owner_record{};
    bool registered{};
};

// Complete 535320 through actual numeric D5F0A8/slot48 B318B0. One persistent
// cache/loader frame per call; canonical cache owners and material owners must
// be identical. Same allocation, raw-slot-only constructor unwind and normal
// effect release as the callable interface below. Acquired fields preserve
// outstanding effects on failure. A completed material is NOT registered here.
NativeMaterialStorage* create_native_material_from_effect_cache_00535320(
    NativeString&, NativeMaterialSlotPool&, NativeRenderActualOwners&,
    NativeMaterialEffectCacheContext&, NativeMaterialFactoryAcquired&);

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
