#pragma once

#include <cstddef>
#include <cstdint>

namespace bsp {

// Native eight-byte cache entry: a mip DWORD and a borrowed surface-owner
// identity. Storage operations do not retain, release, or destroy that owner.
struct NativeTextureSurfaceCacheEntry {
    std::uint32_t mip_00;
    void* surface_owner_04;
};

// Actual header embedded at native texture owner+40h. No constructor,
// destructor, hidden allocation state, or replacement count is supplied.
struct NativeTextureSurfaceCacheStorage {
    NativeTextureSurfaceCacheEntry* data_00;
    std::int32_t count_04;
    std::int32_t capacity_08;
};
static_assert(sizeof(void*) == 4);
static_assert(sizeof(NativeTextureSurfaceCacheEntry) == 8);
static_assert(offsetof(NativeTextureSurfaceCacheEntry, surface_owner_04) == 4);
static_assert(sizeof(NativeTextureSurfaceCacheStorage) == 12);
static_assert(offsetof(NativeTextureSurfaceCacheStorage, count_04) == 4);
static_assert(offsetof(NativeTextureSurfaceCacheStorage, capacity_08) == 8);

// Original ECX=header; signed requested capacity/count stack DWORD; RET4.
// Reserve clamps request to >=1, compares signed capacity, allocates DWORD-
// wrapped capacity*8 bytes, copies current records with sequential DWORD
// loads/stores, frees current old data, then publishes new data/capacity.
// Resize preserves current-field reloads and repeated shrink decrements.
// The original allocator may throw before publication; no EH cleanup added.
void reserve_native_texture_surface_cache_00b3d9b0(
    NativeTextureSurfaceCacheStorage&, std::int32_t requested_capacity);
void resize_native_texture_surface_cache_00b3da20(
    NativeTextureSurfaceCacheStorage&, std::int32_t requested_count);

// Original ECX=header, RET. Resize0, then free current data. The resulting
// count, dangling data pointer, and capacity are retained; no surface release.
void destroy_native_texture_surface_cache_00b3ec40(
    NativeTextureSurfaceCacheStorage&);

// Arithmetic and comparisons reproduce native signed/DWORD behavior without
// validating headers. Every accessed record must be readable/writable and each
// freed pointer an actual shared allocation. Wrapped or negative requests do
// not grant storage for the resulting nominal capacity/count. Concurrent
// mutation, arbitrary invalid memory, and reuse after destroy are not promised.
} // namespace bsp
