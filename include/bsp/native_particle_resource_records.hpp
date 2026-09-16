#pragma once
#include <cstdint>

namespace bsp {
struct NativeStringRawPoolContext;
struct NativeParticleResourceCacheContext;

// Actual record is 2Ch: name {length+0,data+4}, untouched word+8,
// alias sentinel+C/count+10, five payload words+14..24, resource+28.
// Actual vector is {data+0,signed count+4,signed capacity+8}; cache embeds it+4.
// All entries use the genuine raw string-pool publications and current CRT
// allocation/validation services. C++ APIs are not original ABI replacements.
// Full native spans, original ABI, cleanup states and limits are in
// docs/NATIVE_PARTICLE_RESOURCE_RECORDS_ORCH4.md.

// ECX record, RET. Destroy aliases/sentinel then name; retain other bytes.
void destroy_native_particle_resource_record_0086fcf0(void*, NativeStringRawPoolContext&);
// ECX destination, stack source, EAX destination, RET4. Constructor, not assign.
void* copy_construct_native_particle_resource_record_0086ff50(
    void* destination, const void* source, NativeStringRawPoolContext&);
// ECX vector, stack requested capacity, RET4. Minimum40h, signed comparisons.
void reserve_native_particle_resource_records_00870000(
    void* vector, std::int32_t capacity, NativeStringRawPoolContext&);
// ECX vector, stack source record, RET4. Copy only then increment current count.
void append_native_particle_resource_record_00870ac0(
    void* vector, const void* source, NativeStringRawPoolContext&);
// ECX vector, stack count, RET4. New records preserve+8/+28 and prior raw bytes.
void resize_native_particle_resource_records_00870b30(
    void* vector, std::int32_t count, NativeStringRawPoolContext&);
// ECX cache owner, RET. Current vslot10 releases last resource before current
// last-record destruction. Known D0DAF0/D0DB40 identities compose the actual
// 871420 provider; other tables must contain callable thiscall slot10 targets.
void clear_native_particle_resource_cache_00871310(void*, NativeStringRawPoolContext&);

// Same clear algorithm, composing known resource identities through actual
// cache and string contexts. The older callable-vtable overload is preserved.
void clear_native_particle_resource_cache_00871310(void*, NativeParticleResourceCacheContext&, NativeStringRawPoolContext&);

}
