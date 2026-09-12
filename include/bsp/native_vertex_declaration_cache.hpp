#pragma once

#include "bsp/native_render_resource_record.hpp"
#include <cstdint>

namespace bsp {
class ActualNativeStringPoolStorage;
struct NativeVertexDeclarationLoadingContext;
struct NativeVfsDateRouteContext;
struct ResourceLoadEventHost;
struct SingletonLifetimeCallbacks;

// One actual D5F060 declaration registry, normally embedded at renderer+1A60.
// +04/+08/+0C are its actual 2Ch-record array; +10 accumulates loaded strides.
// Reuse NativeRenderResourceRecord's producer-verified storage, without a
// separate cache, owner map, or resource-reference operation on record copies.
struct NativeVertexDeclarationCacheContext {
    ActualNativeStringPoolStorage& strings;
    const SingletonLifetimeCallbacks& validation;
    NativeVertexDeclarationLoadingContext& declarations;
    NativeVfsDateRouteContext& dates;
    // Canonical application platform projection at the current0109CF04
    // publication. It must dispatch that owner's real XLive/cursor services.
    // This pointer is not overlaid onto the original platform's raw storage.
    ResourceLoadEventHost* const volatile& platform_0109cf04;
    const volatile std::uint32_t* registry_vtable_00d5f060;
    void* (*allocate_array_00bf55be)(std::uint32_t bytes);
    void (*free_array_00bf6989)(void*) noexcept;
};

// Full B2FBB0; ECX destination, stack source, EAX destination, RET4.
// Initial name copy is outside its cleanup; aliases arm only name cleanup.
NativeRenderResourceRecord& copy_construct_native_declaration_record_00b2fbb0(
    NativeRenderResourceRecord&, const NativeRenderResourceRecord&,
    ActualNativeStringPoolStorage&, const SingletonLifetimeCallbacks&);
// Full B2F910..B2F987 including returning-free continuation. ECX record, RET.
// Destroy aliases/current sentinel, null +0C, release current name, leave all
// other fields and the unretained resource+28 unchanged.
void destroy_native_declaration_record_00b2f910(NativeRenderResourceRecord&,
    ActualNativeStringPoolStorage&);
// Actual 0Ch vector headers; signed comparisons and DWORD arithmetic retained.
// Native ECX header, stack requested capacity / record, RET4. Copy failure
// preserves the native no-op placement-delete unwind, without added rollback.
void reserve_native_declaration_records_00b2fe20(void*, std::uint32_t,
    NativeVertexDeclarationCacheContext&);
void append_native_declaration_record_00b300c0(void*,
    const NativeRenderResourceRecord&, NativeVertexDeclarationCacheContext&);

// B2C280: incoming ECX unused; stack output/name/ignored, EAX output, RET0C.
// Clears actual8h output before identity comparison; no old-output release.
void* copy_native_declaration_resolved_name_00b2c280(void* actual_output,
    const void* actual_name, ActualNativeStringPoolStorage&);
// B31D20: incoming ECX unused; stack actual declaration, EAX same, RET4.
// One InterlockedIncrement on that owner's existing +04; no companion retain.
void* acquire_native_cached_declaration_00b31d20(void* actual_declaration) noexcept;

// Complete B305F0 schedule, qualified to original D5F060 +04/+08/+0C slots.
// ECX registry; stack name/loader-word/acquire-new/allow-load; EAX owner;
// RET10h. Hit ALWAYS acquires; only fresh results honor acquire_new's low byte.
// A failed load still appends a null record and queries its actual VFS date.
// All native list searches, alias insertion and current publication reads are
// preserved. Different registry profiles are explicit source-domain errors.
void* load_native_cached_vertex_declaration_00b305f0(void* actual_registry,
    const void* actual_name, std::uint32_t loader_word,
    std::uint8_t acquire_new, std::uint8_t allow_load,
    NativeVertexDeclarationCacheContext&);
// Full B317E0; ECX renderer; stack name; EAX declaration; RET4. Actual header
// copy/lowercase, embedded registry(name,0,1,1), captured-data normal cleanup.
void* load_native_renderer_vertex_declaration_00b317e0(void* actual_renderer,
    const void* actual_name, NativeVertexDeclarationCacheContext&);

// New C++ interfaces. Host C++ exceptions/current CRT string comparisons;
// original FH3/SEH and binary ABI are not replaced. Dependencies must belong
// to the same actual string and declaration domains. No game validation.
} // namespace bsp
