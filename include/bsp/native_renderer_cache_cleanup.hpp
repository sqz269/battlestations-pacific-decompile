#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native renderer cache cleanup requires MSVC Win32.
#endif

namespace bsp {
class ActualNativeStringPoolStorage;
class NativeRenderActualOwners;
struct SingletonLifetimeCallbacks;
struct NativeRenderResourceAccountingTables;
struct NativeMaterialEffectCacheContext;
struct NativeEffectRecordStorageContext;

// Optional Source-only diagnostics. A resource may have reached its terminal
// before an after-release observation; prefix values are OS memory snapshots,
// not an assertion of allocation liveness. Observers must not mutate or unwind
// the native cleanup and must not retain a reference to the callback frame.
enum class NativeRendererCacheCleanupEvent {
    clear_enter, child_release_before, child_release_after, clear_exit,
    fallback_release_before, fallback_release_after
};
struct NativeRendererCacheCleanupObservation {
    std::uint32_t native_function{};
    NativeRendererCacheCleanupEvent event{};
    void* cache{};
    void* resource{};
    std::uint32_t cache_count{};
    void* fallback{};
    bool fallback_read{};
    bool resource_prefix_read{};
    std::uint32_t snapshot_profile{};
    std::uint32_t snapshot_reference_bits{};
};
using NativeRendererCacheCleanupObserver = void (*)(void*,
    const NativeRendererCacheCleanupObservation&);

// Source-only record (packet cc15-rshut) of B316C0's own skipped release. When
// the release at B316FC reaches a zero terminal whose B32250/B31DC0 callback
// removes the last row, B316FE rereads the count and B3170B/B3170F destroy the
// row below it unreleased. B316C0 records that row's resource and name here and
// sets b316c0_ran when it returns. NativeTextureLoadOwners reads it at exit.
struct NativeRendererCacheSkippedRelease {
    bool b316c0_ran{};
    std::uint32_t skipped{};
    void* resource{};
    char name[96]{};
};

// Borrow the application's SAME pool, canonical actual-reference owners and
// current numeric profile tables. No cache/owner/count copy is made. Selected
// cache tables require slot+10=B31DA0; child tables require their known +0C
// accounting leaf. Unsupported profiles/slots are outside this source domain.
struct NativeRendererCacheCleanupContext {
    ActualNativeStringPoolStorage& strings;
    const SingletonLifetimeCallbacks& validation;
    NativeRenderActualOwners& owners;
    const NativeRenderResourceAccountingTables& accounting_tables;
    const volatile std::uint32_t* cache_base_vtable_00d5f038;
    const volatile std::uint32_t* texture_cache_vtable_00d5f088;
    void* observation_context{};
    NativeRendererCacheCleanupObserver observe_cleanup{};
    NativeRendererCacheSkippedRelease* skipped_release{};
};

// B30410[204], ECX raw0Ch record-array header, stack count, RET4. Full growth
// and shrink body using actual effect-record reserve/storage destruction.
// Growth preserves row+08/+28; failure cleans only current partial name.
void resize_native_effect_records_00b30410(void* actual_header,
    std::uint32_t requested_count, NativeMaterialEffectCacheContext&);
void resize_native_effect_records_00b30410(void* actual_header,
    std::uint32_t requested_count, NativeEffectRecordStorageContext&);

// B316C0[106], ECX actual registry, RET. +04 data,+08 count,+0C capacity,
// +10 accounting. Each iteration dispatches current last child+0C, subtracts
// size, reloads child and current cache+10, then destroys the current last row
// before decrementing current count. Finish with actual record-array resize0.
void clear_native_renderer_resource_cache_00b316c0(
    void* actual_registry, NativeRendererCacheCleanupContext&);

// B31730[23], ECX raw0Ch header, RET. Resize0, free current data, retain stale
// pointer/capacity. Shared by the base destructor's exact state0 cleanup.
void destroy_native_renderer_resource_array_00b31730(
    void* actual_header, NativeRendererCacheCleanupContext&);

// B32090[95], ECX registry, RET. Publish D5F038, clear cache with array cleanup
// armed, then disarm before normal resize0/free. Does not free the registry.
void destroy_native_renderer_resource_cache_00b32090(
    void* actual_registry, NativeRendererCacheCleanupContext&);

// B32370[146], ECX registry, RET. Publish D5F088; release captured +20 through
// actual+04/current-profile terminal then clear+20; return captured name+18
// with current+14 length+1, preserving header bytes; finally base destruction.
// State1 unwind returns current name then destroys base; state0 destroys base.
void destroy_native_texture_resource_cache_00b32370(
    void* actual_registry, NativeRendererCacheCleanupContext&);

// New C++ interfaces: original private-frame/SEH and binary caller ABI are not
// claimed. Native raw extents and valid reached profiles are preconditions.
// Source unwinding preserves the observed armed cleanup schedule. Failure of
// cleanup while already unwinding terminates; actual-pool release and canonical
// terminal retain their existing noexcept obligations. No game validation.
} // namespace bsp
