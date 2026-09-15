#pragma once
#include <cstdint>
#include <memory>

namespace bsp {
struct NativeGameResourceFactoryContext;
struct NativeResourceManagerContext;
struct NativeResourceLoadCacheContext;
class NativeResourceLoadCacheAcquired;

struct NativeGameResourceLoadContext {
    NativeGameResourceFactoryContext& factory;
    NativeResourceManagerContext& manager;
    NativeResourceLoadCacheContext& cache;
};
// Full28-byte7188A0: ECX actual name, EAX resource, RET. Capture factory
// before manager getter; call fullB80720 with that factory and original name.
void* load_native_game_resource_007188a0(const void* actual_name,
    NativeGameResourceLoadContext&, NativeResourceLoadCacheAcquired&);

class NativeDamageableClassModelCalls {
public:
    virtual ~NativeDamageableClassModelCalls() = default;
    virtual std::uint32_t class_slot20(std::uint32_t current_profile) noexcept = 0;
    virtual void bind_model(std::uint32_t captured_target, void* actual_class) = 0;
};
struct NativeDamageableClassModelContext {
    NativeGameResourceLoadContext& resources;
    NativeDamageableClassModelCalls& classes;
};
enum class NativeDamageableClassModelPhase { fresh, running, complete, failed };
// One invocation owns only native temporary headers and the existing nested
// VFS/cache invocation frames. No descriptor, resource, registry or pool copy.
// Retain this frame if a nested resolver fails under its existing failure-frame
// contract. It cannot be replayed. All borrowed contexts outlive the frame.
class NativeDamageableClassModelAcquired final {
public:
    explicit NativeDamageableClassModelAcquired(NativeDamageableClassModelContext&);
    ~NativeDamageableClassModelAcquired();
    NativeDamageableClassModelAcquired(const NativeDamageableClassModelAcquired&) = delete;
    NativeDamageableClassModelAcquired& operator=(const NativeDamageableClassModelAcquired&) = delete;
    NativeDamageableClassModelPhase phase() const noexcept;
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
    friend void load_native_damageable_class_model_00879590(void*, std::uint32_t,
        NativeDamageableClassModelContext&, NativeDamageableClassModelAcquired&);
};
// Full533-byte879590: ECX class, low byte of stacked enemy flag, RET4.
// Name38 length must be nonzero and resource50 null. Optionally insert _enemy
// before FIRST dot and replace the class name only after actual VFS success.
// Full7188A0 result is published at50 BEFORE load-name temporary cleanup.
void load_native_damageable_class_model_00879590(void* actual_class,
    std::uint32_t enemy, NativeDamageableClassModelContext&, NativeDamageableClassModelAcquired&);
// Full36-byte879AA0: call879590, reload50, dispatch CURRENT class slot20 if
// nonnull, then set byte44 to1. No resource retain or bind/activation rollback.
void activate_native_damageable_class_model_00879aa0(void*, std::uint32_t enemy,
    NativeDamageableClassModelContext&, NativeDamageableClassModelAcquired&);
// Full14-byte43EBD0: existing nonzero byte44 returns, otherwise tail879AA0.
void ensure_native_damageable_class_model_0043ebd0(void*, std::uint32_t enemy,
    NativeDamageableClassModelContext&, NativeDamageableClassModelAcquired&);

// Explicit MSVC Win32 source interfaces, not original ABI/FH3/SEH bridges.
// Same raw string publications as resource manager; substring/concat/assignment
// retain their existing NativeStringStorage noexcept-release boundary. Actual
// class slot20 implementations remain required. No null-model fallback is added.
} // namespace bsp
