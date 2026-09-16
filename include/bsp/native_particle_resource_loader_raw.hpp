#pragma once
#include <cstdint>
#include <memory>

namespace bsp {
struct NativeStringRawPoolContext;
struct NativeVfsNameResolutionContext;
class NativeVfsNameResolutionAcquired;
class NativeVfsRuntimeBindings;
struct NativeParticleResourceParserRawContext;
struct NativeParticleResourceParserRawAcquired;

// Borrow the SAME actual raw string cells as the VFS and particle parser,
// the live VFS publication and its concrete services. The immutable empty
// name is the actual image's F8766C string. No fallback VFS or parser is added.
struct NativeParticleResourceLoaderRawContext {
    NativeStringRawPoolContext& strings;
    void* volatile& actual_vfs_publication_0109ceec;
    NativeVfsRuntimeBindings& vfs;
    NativeVfsNameResolutionContext& name_resolution;
    NativeParticleResourceParserRawContext& parser;
    const char* empty_name_00f8766c;
};

enum class NativeParticleResourceLoaderRawPhase { fresh, running, complete, failed };
// One immovable invocation. Actual 8h copied name and 1Ch text buffer precede
// retained resolver/parser children. Native cleanup preserves stale headers;
// destruction never replays cleanup or releases a constructed resource.
// Retain failed frames and borrowed contexts until provider obligations resolve
// (the existing failed VFS frames currently require process lifetime).
class NativeParticleResourceLoaderRawAcquired final {
public:
    // Explicit incoming parser builder+C; distinct from its nested child kind.
    explicit NativeParticleResourceLoaderRawAcquired(std::int32_t incoming_parser_builder_kind);
    ~NativeParticleResourceLoaderRawAcquired();
    NativeParticleResourceLoaderRawAcquired(const NativeParticleResourceLoaderRawAcquired&) = delete;
    NativeParticleResourceLoaderRawAcquired& operator=(const NativeParticleResourceLoaderRawAcquired&) = delete;
    NativeParticleResourceLoaderRawPhase phase() const noexcept;
    std::uint32_t failure_site() const noexcept;
    std::int32_t native_state_at_failure() const noexcept;
    void* constructed_resource() const noexcept;
    const NativeVfsNameResolutionAcquired* resolution_invocation() const noexcept;
    const NativeParticleResourceParserRawAcquired* parser_invocation() const noexcept;
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
    friend void* create_native_particle_resource_0086ba60(void*, const void*, std::uint32_t,
        NativeParticleResourceLoaderRawContext&, NativeParticleResourceLoaderRawAcquired&);
};

// Complete 0086BA60[278]. Incoming ECX is ignored. TWO stacked arguments:
// actual8h name and ignored second DWORD; RET8/EAX resource. Copy/lowercase,
// allocate90h, genuine AF45D0 and D0D418 stamp, AF5850, AF4BA0, native cleanup.
// Both AF5850 count and AF4BA0 AL are ignored. Null allocation still reaches
// the native parser dereference. Constructed resources survive load/parse
// failure; constructor failure frees only its allocation after inner unwind.
// New source interface, not original register/FH3/hardware-SEH or game proof.
void* create_native_particle_resource_0086ba60(void* ignored_cache,
    const void* actual_name, std::uint32_t ignored_second,
    NativeParticleResourceLoaderRawContext&, NativeParticleResourceLoaderRawAcquired&);
} // namespace bsp
