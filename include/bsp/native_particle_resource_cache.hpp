#pragma once
#include <cstdint>

namespace bsp {
struct NativeStringRawPoolContext;

// Borrow the application's actual manager and particle-cache publication cells.
// No alternate owner, allocator, registration list or resource-loader service.
struct NativeParticleResourceCacheContext {
    void* volatile& actual_manager_publication_01090aa0;
    void* volatile& actual_cache_publication_00f87668;
};

// 871400/871420: incoming ECX ignored, stack resource, RET4. Retain returns
// that resource in EAX. Release decrements actual LONG+4 and, only on zero,
// calls CURRENT resource vtable slot0 with ECX=resource and NO flags argument.
// The resource owns a real callable Win32 vtable; it is not a cache record.
void* retain_native_particle_resource_00871400(void* actual_resource);
void release_native_particle_resource_00871420(void* actual_resource);

// 86BA10: incoming owner ECX ignored; stacked output8h, source8h, ignored
// third argument; EAX output, RET0Ch. Zero output BEFORE the self test. No
// previous-output cleanup. Reload source and destination after raw resize.
void* construct_native_particle_resource_cache_key_0086ba10(
    void* actual_owner, void* actual_output, const void* actual_source,
    const void* unused_third_argument, NativeStringRawPoolContext&);

// 871370: ECX actual12h vector, RET. Resize0 then free its CURRENT data.
// 871480: ECX actual14h inner owner, RET. Stamp D0DAF0; clear through871310;
// destroy vector at+4. While clear throws, true unwind destroys that vector;
// a second exception from that cleanup terminates.
void destroy_native_particle_resource_records_00871370(
    void* actual_vector, NativeStringRawPoolContext&);
void destroy_native_particle_resource_cache_00871480(
    void* actual_inner, NativeStringRawPoolContext&);
// ECX inner, stack flags, EAX original address, RET4; free only for bit0.
void* delete_native_particle_resource_cache_00871730(
    void* actual_inner, std::uint32_t flags, NativeStringRawPoolContext&);

// 86A200: ECX outer owner, RET. Clear publication then stamp CE3818.
// 871AE0: ECX outer18h owner, RET. Destroy inner+4, then the base above;
// the base cleanup also runs while inner destruction unwinds.
void destroy_native_particle_resource_cache_base_0086a200(
    void* actual_outer, NativeParticleResourceCacheContext&) noexcept;
void destroy_native_particle_resource_cache_owner_00871ae0(
    void* actual_outer, NativeParticleResourceCacheContext&,
    NativeStringRawPoolContext&);
void* delete_native_particle_resource_cache_owner_00871b30(
    void* actual_outer, std::uint32_t flags,
    NativeParticleResourceCacheContext&, NativeStringRawPoolContext&);

// 871BD0: no native input, EAX current outer publication, RET. Capture actual
// manager+10 section, enter/increment, recheck, allocate18h and initialize,
// publish, fetch manager AGAIN, reread publication and register. Registration
// failure retains publication. The final slow-path read occurs after Leave.
void* get_native_particle_resource_cache_owner_00871bd0(
    NativeParticleResourceCacheContext&);

// Profiles D0DAF0/D0DB40/D0DB54 remain original identity DWORDs, not source
// callable vtables. 86BA60 (resource construction/loading/parsing) is absent:
// no complete cache virtual surface or executable resource-acquisition path
// is claimed. These C++ entry points do not reproduce FH3/SEH stack identity,
// hardware-fault cleanup, original CRT domain or the original callable ABI.
} // namespace bsp
