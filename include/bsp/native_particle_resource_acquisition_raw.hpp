#pragma once
#include <cstdint>
#include <memory>

namespace bsp {
struct NativeStringRawPoolContext;
struct NativeParticleResourceLoaderRawContext;
class NativeParticleResourceLoaderRawAcquired;
struct NativeVfsDateRouteContext;
struct ResourceLoadEventHost;
struct SingletonLifetimeCallbacks;

// Borrow the SAME string/VFS/parser domains used by loading and destruction.
// Profile views contain actual original DWORD targets through slot+C; numeric
// targets are decoded to proved source bodies, never called as host functions.
// The load-event host must bind the named actual platform owner. A different
// current publication is an explicit source binding boundary, not a fallback.
struct NativeParticleResourceAcquisitionRawContext {
    NativeStringRawPoolContext& strings;
    NativeParticleResourceLoaderRawContext& loader;
    NativeVfsDateRouteContext& dates;
    void* volatile& actual_vfs_publication_0109ceec;
    void* volatile& actual_platform_publication_0109cf04;
    const void* bound_platform_identity;
    ResourceLoadEventHost& load_events;
    const void* actual_cache_profile_00d0db40;
    const void* actual_base_profile_00d0daf0;
    const SingletonLifetimeCallbacks& crt;
};

enum class NativeParticleResourceAcquisitionRawPhase { fresh, running, complete, failed };
class NativeParticleResourceAcquisitionRawAcquired final {
public:
    explicit NativeParticleResourceAcquisitionRawAcquired(std::int32_t incoming_parser_builder_kind);
    ~NativeParticleResourceAcquisitionRawAcquired();
    NativeParticleResourceAcquisitionRawAcquired(const NativeParticleResourceAcquisitionRawAcquired&) = delete;
    NativeParticleResourceAcquisitionRawAcquired& operator=(const NativeParticleResourceAcquisitionRawAcquired&) = delete;
    NativeParticleResourceAcquisitionRawPhase phase() const noexcept;
    std::uint32_t failure_site() const noexcept;
    std::int32_t native_state_at_failure() const noexcept;
    std::uint32_t captured_profile() const noexcept;
    std::uint32_t captured_target() const noexcept;
    void* loaded_resource() const noexcept;
    void* captured_platform() const noexcept;
    const NativeParticleResourceLoaderRawAcquired* loader_invocation() const noexcept;
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
    friend void* acquire_native_particle_resource_00870dd0(void*, const void*,
        std::uint32_t, std::uint32_t, std::uint32_t,
        NativeParticleResourceAcquisitionRawContext&, NativeParticleResourceAcquisitionRawAcquired&);
};

// Complete 1338-byte body. Native ECX inner cache, stack name/forwarded word/
// retain-new/allow-load, EAX resource, RET10. Only low bytes of flags are read.
// Cache hits always retain; new loads retain only when requested. A failed
// later operation never rolls back a loaded resource or an inserted alias.
// Actual 23-DWORD frame precedes a retained loader invocation. Keep failed
// frames/domains alive through provider obligations (failed VFS: process life).
// Destruction never retries native cleanup. New C++ ABI; no FH3/SEH/game claim.
void* acquire_native_particle_resource_00870dd0(void* actual_cache,
    const void* actual_name, std::uint32_t forwarded_word, std::uint32_t retain_new,
    std::uint32_t allow_load, NativeParticleResourceAcquisitionRawContext&,
    NativeParticleResourceAcquisitionRawAcquired&);
} // namespace bsp
