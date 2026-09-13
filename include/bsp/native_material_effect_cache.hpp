#pragma once

#include "bsp/native_material_effect_loading.hpp"
#include "bsp/native_render_resource_record.hpp"
#include <cstdint>

namespace bsp {
class ActualNativeStringPoolStorage;
struct NativeVfsDateRouteContext;
struct ResourceLoadEventHost;
struct SingletonLifetimeCallbacks;
struct NativeRendererSynchronizationGlobals;

// Actual D5F074 registry, normally renderer+1A98. +4/+8/+C is the actual
// twelve-byte vector of 2Ch records; +10 is DWORD size accounting. Every
// borrowed table contains original numeric targets, never host callables.
struct NativeMaterialEffectCacheContext {
    ActualNativeStringPoolStorage& strings;
    const SingletonLifetimeCallbacks& validation;
    NativeMaterialEffectLoadingContext& effects;
    NativeVfsDateRouteContext& dates;
    ResourceLoadEventHost* const volatile& platform_0109cf04;
    NativeRendererSynchronizationGlobals& synchronization_0108d6dc;
    const volatile std::uint32_t* registry_vtable_00d5f074;
    const volatile std::uint32_t* effect_base_vtable_00d5e534;
    const volatile std::uint32_t* effect_derived_vtable_00d61a00;
    void* (*allocate_array_00bf55be)(std::uint32_t bytes);
    void (*free_array_00bf6989)(void*) noexcept;
};

// Caller-owned operation frame, not an owner map or a second reference count.
// Keep it through failure and resolve its loader's actual acquired state.
// loaded is the loader's returned owned reference, including recursive cache
// fallback; it is NOT necessarily a new object or a reference count of one.
// Once cache_published, that reference belongs to the actual record. A caller
// acquisition is additional, and survives any subsequent cleanup failure.
// Frames cannot replay after starting; no destructor releases native owners.
struct NativeMaterialEffectCacheAcquired {
    enum class Phase { not_started, running, complete, failed };
    NativeMaterialEffectCacheAcquired() = default;
    NativeMaterialEffectCacheAcquired(const NativeMaterialEffectCacheAcquired&) = delete;
    NativeMaterialEffectCacheAcquired& operator=(const NativeMaterialEffectCacheAcquired&) = delete;
    Phase phase{Phase::not_started};
    std::uint32_t native_site{};
    NativeMaterialEffectLoadAcquired loader;
    void* loaded{};
    void* result{};
    bool cache_published{};
    bool caller_acquired{};
};

// ECX destination, stack source, EAX destination, RET4. Copies actual names,
// aliases, five date DWORDs, and unretained resource+28. Leaves +08 untouched.
NativeRenderResourceRecord& copy_construct_native_effect_record_00b2fd10(
    NativeRenderResourceRecord&, const NativeRenderResourceRecord&,
    ActualNativeStringPoolStorage&, const SingletonLifetimeCallbacks&);
// ECX record, RET; complete returning-free tail through B2FA87. Releases only
// aliases/sentinel/name, nulls +0C, and does not release resource+28.
void destroy_native_effect_record_00b2fa10(NativeRenderResourceRecord&,
    ActualNativeStringPoolStorage&);
// ECX actual0Ch vector header, stack capacity/source, RET4. Native signed
// bounds, live reloads, DWORD wrapping and no-op placement-delete unwind.
void reserve_native_effect_records_00b2ffe0(void*, std::uint32_t,
    NativeMaterialEffectCacheContext&);
void append_native_effect_record_00b301a0(void*, const NativeRenderResourceRecord&,
    NativeMaterialEffectCacheContext&);
// Incoming ECX unused; stack output/name/ignored-word, EAX output, RET0C.
// Replace EVERY case-sensitive .mshd by .shfx; resolve changed first, otherwise
// resolve original and return its mutable result even when resolution fails.
void* resolve_native_effect_cache_name_00b2e940(void* actual_output,
    const void* actual_name, NativeMaterialEffectCacheContext&);
// ECX unused, stack nonnull actual effect, EAX same pointer, RET4. One actual
// InterlockedIncrement(+04), without a companion or hidden owner reference.
void* acquire_native_cached_effect_00b31ff0(void* actual_effect) noexcept;

// ECX registry, four stack DWORDs, EAX effect, RET10. Hit ALWAYS acquires;
// fresh load honors acquire_new low byte. allow_load low byte only gates the
// loader AFTER alias resolution. Null loads still append records/query dates.
// A cold load requires retained acquired output BEFORE invoking B2EBB0;
// nullptr supports hit/no-load paths, with an explicit cold-path exception.
void* load_native_cached_material_effect_00b31090(void* actual_registry,
    const void* actual_name, std::uint32_t loader_word, std::uint8_t acquire_new,
    std::uint8_t allow_load, NativeMaterialEffectCacheContext&,
    NativeMaterialEffectCacheAcquired* acquired = nullptr);
// ECX renderer, stack name, EAX effect, RET4. Captures receiver separately
// from the current renderer publication used for optional guard entry. Current
// mode governs cleanup; skipped-entry then enabled-exit is native undefined
// guard preimage, outside the supported source interface. Fixed 0/1/1 flags.
void* load_native_renderer_material_effect_00b318b0(void* actual_renderer,
    const void* actual_name, NativeMaterialEffectCacheContext&,
    NativeMaterialEffectCacheAcquired* acquired = nullptr);

// New C++ interfaces with explicit same-domain providers; no native SEH/ABI,
// complete actual effect-loader composition, or game validation claim.
} // namespace bsp
