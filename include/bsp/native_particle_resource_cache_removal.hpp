#pragma once
#include <cstdint>
namespace bsp {
struct NativeStringRawPoolContext;
struct NativeParticleResourceCacheContext;
// 870C10 ECX actual2Ch destination, stack source, EAX destination, RET4.
// Copy current name, replace alias list, copy all6tail words. No resource refcount.
void* assign_native_particle_resource_record_00870c10(void*, const void*, NativeStringRawPoolContext&);
// 8714E0 ECX cache inner {vptr,data,count,capacity}, stack name8h, RET4.
// Search original copied name after constructing/destroying a normalized scratch.
// Replace a matched nonlast record with current last, destroy current last and
// decrement current count. No resource release; current CRT validation may return.
void remove_native_particle_resource_by_alias_008714e0(void*, const void*, NativeStringRawPoolContext&);
// Actual final loader profileD0D418: slot0BD30E0 ->slot4scalar871FA0 ->871CA0.
// The derived destructor removes its name through actual cache publications,
// then destroysAF4280. Base cleanup is a true unwind action on removal failure.
void destroy_native_cached_particle_resource_00871ca0(void*, NativeParticleResourceCacheContext&, NativeStringRawPoolContext&);
void* delete_native_cached_particle_resource_00871fa0(void*, std::uint32_t flags,
    NativeParticleResourceCacheContext&, NativeStringRawPoolContext&);
}
