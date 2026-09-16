#pragma once
#include <cstdint>
#include <memory>

namespace bsp {
struct NativeResourceManagerContext;
struct NativeResourceLoadCacheContext;
class NativeResourceLoadCacheAcquired;
class NativeVfsNameResolutionAcquired;
class NativeResourceContainerReferences;

// Borrow the application's SAME actual manager, VFS, parser, string and terminal
// domains. Numeric profile data must be readable as required by these providers.
struct NativeParticleObjectResourcesRawContext {
    NativeResourceManagerContext& manager;
    NativeResourceLoadCacheContext& cache;
    NativeResourceContainerReferences& references;
    void* volatile& factory_alias_00f8d31c;
    const char* empty_stem_00f8d320;
};

enum class NativeParticleObjectResourcesRawPhase { fresh, running, complete, failed };
// One immovable call frame. Headers and both native 1024-byte buffers precede
// retained child invocations. A failed child and this parent must stay alive;
// never reset/replay after a consumed decrement, publication or provider failure.
class NativeParticleObjectResourcesRawAcquired final {
public:
    NativeParticleObjectResourcesRawAcquired();
    ~NativeParticleObjectResourcesRawAcquired();
    NativeParticleObjectResourcesRawAcquired(const NativeParticleObjectResourcesRawAcquired&) = delete;
    NativeParticleObjectResourcesRawAcquired& operator=(const NativeParticleObjectResourcesRawAcquired&) = delete;
    NativeParticleObjectResourcesRawPhase phase() const noexcept;
    std::uint32_t failure_site() const noexcept;
    std::int32_t native_state_at_failure() const noexcept;
    const NativeVfsNameResolutionAcquired* resolution_invocation() const noexcept;
    const NativeResourceLoadCacheAcquired* cache_invocation() const noexcept;
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
    friend void load_native_object_particle_models_00af9660(void*, const char*,
        NativeParticleObjectResourcesRawContext&, NativeParticleObjectResourcesRawAcquired&);
};

// AF8940: ECX actual98h Object, RET. Reverse current-count traversal, one actual
// interlocked +4 decrement, captured current slot0 through real container chain.
// Clear the captured cell only after normal terminal return; reload current count.
void clear_native_object_particle_models_00af8940(void*, NativeResourceContainerReferences&);
// B80D70: ECX actual manager; stack name8h; RET4/EAX. Capture current manager+4
// then call actual B80720. The caller owns the one-shot cache invocation.
void* load_native_resource_with_default_factory_00b80d70(void*, const void*,
    NativeResourceLoadCacheContext&, NativeResourceLoadCacheAcquired&);
// AF9660: ECX actual98h Object; stack filename; RET4. Whole filename-sequence,
// actual VFS/cache composition and native eleven-state cleanup, no owner rollback.
void load_native_object_particle_models_00af9660(void*, const char*,
    NativeParticleObjectResourcesRawContext&, NativeParticleObjectResourcesRawAcquired&);
// New source interfaces. No native register/FH3/hardware-fault ABI guarantee,
// arbitrary missing parser/terminal fallback, or game-runtime validation.
} // namespace bsp
