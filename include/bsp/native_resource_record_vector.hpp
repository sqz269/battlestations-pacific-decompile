#pragma once

#include "bsp/native_resource_record_copy.hpp"

namespace bsp {

// Actual 12-byte/0Ch header. Counts retain DWORD bits; native comparisons are
// signed. The data pointer owns raw storage containing actual2Ch records.
struct NativeResourceRecordVectorStorage {
    NativeRenderResourceRecord* data_00;
    std::uint32_t count_04;
    std::uint32_t capacity_08;
};

// Full004DA180..004DA25D: ECX header, stack requested capacity, RET4.
// Signed minimum64; DWORD allocation product. Copy current old records, then
// destroy current old records, free current base, publish new base/capacity.
// A failed copy leaves the new allocation and completed prefix owned/orphaned
// as in the original. There is no completed-prefix rollback or invented guard.
void reserve_native_resource_record_vector_004da180(
    NativeResourceRecordVectorStorage& actual_header,
    std::int32_t requested_capacity,
    ActualNativeStringPoolStorage& actual_string_pool,
    const SingletonLifetimeCallbacks& actual_validation);

// Full00B1A3C0..00B1A42F: ECX header, stack source record pointer, RET4.
// Grow only on count==capacity; wrap doubling then apply signed minimum64.
// Recompute the current destination, copy if its address is nonzero, then
// increment current count. Source is read only after reserve and only for a
// nonzero destination. The append unwind rereads current count then data.
void append_native_resource_record_00b1a3c0(
    NativeResourceRecordVectorStorage& actual_header,
    const NativeRenderResourceRecord* actual_source,
    ActualNativeStringPoolStorage& actual_string_pool,
    const SingletonLifetimeCallbacks& actual_validation);

// Valid actual storage and the existing actual pool/CRT domains are required.
// No source snapshot protects aliases invalidated by growth; no bounds,
// overflow, null-allocation or resource ownership policy is introduced.
// These host C++ interfaces retain the existing record/string EH limits and
// are not original ABI or SEH replacements. C66680/CBC640 are no-op placement
// cleanup dependencies, not additional reconstructed entry claims.

} // namespace bsp
