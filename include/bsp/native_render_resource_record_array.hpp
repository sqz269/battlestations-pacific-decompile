#pragma once

#include "bsp/native_render_resource_record.hpp"

namespace bsp {

// actual_array_header is the native 12-byte {records, count, capacity} header
// at renderer resource-container +4. No owner is copied or constructed here.
// The request and header counts retain native DWORD bits; comparisons are signed.
// Both native methods use ECX=header, one stack DWORD, RET4, no semantic result.
//
// Complete 00B2FF00..00B2FFDD, including post-free publication. Clamp the request
// to signed minimum 64, copy current records into the new raw allocation, then
// destroy current old records, free current storage, publish pointer/capacity.
// A throwing copy leaves the replacement and completed copies allocated.
void reserve_native_render_resource_record_array_00b2ff00(
    void* actual_array_header, std::uint32_t requested_capacity,
    SizedStoragePool& actual_string_pool, const SingletonLifetimeCallbacks&);
void reserve_native_render_resource_record_array_00b2ff00(
    void* actual_array_header, std::uint32_t requested_capacity,
    ActualNativeStringPoolStorage&, const SingletonLifetimeCallbacks&);

// Complete 00B30340..00B3040C. Grow in current storage with actual empty names,
// allocated alias sentinels and descending zero stores at record +24..+14.
// Leave +08 and resource +28 untouched. On growth failure, clean only the
// current partial name; earlier completed records remain outside current count.
// Shrink decrements current count before each current-data record destruction.
void resize_native_render_resource_record_array_00b30340(
    void* actual_array_header, std::uint32_t requested_count,
    SizedStoragePool& actual_string_pool, const SingletonLifetimeCallbacks&);

} // namespace bsp
