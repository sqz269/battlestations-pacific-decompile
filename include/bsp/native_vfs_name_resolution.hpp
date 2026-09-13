#pragma once
#include <cstdint>
#include <memory>

namespace bsp {
struct NativeVfsDeviceRouteContext;
struct NativeVfsOpenLoggingContext;

// Borrow the actual device/lookup/string/CRT domains and live logging gate.
// No manager, registration list/tree, provider, pool or global is copied.
struct NativeVfsNameResolutionContext {
    NativeVfsDeviceRouteContext& device;
    NativeVfsOpenLoggingContext& logging;
    const char* empty_name_0109cef0;
};

enum class NativeVfsNameResolutionPhase { fresh, normalizing, candidates, logging, complete, failed };
// One invocation's native temporary headers/iterators and source diagnostics.
// A failed call cannot replay. Retain this frame and the mutable caller header
// while diagnosing an interrupted provider. Cleanup does not restore the name.
class NativeVfsNameResolutionAcquired final {
public:
    NativeVfsNameResolutionAcquired();
    ~NativeVfsNameResolutionAcquired();
    NativeVfsNameResolutionAcquired(const NativeVfsNameResolutionAcquired&) = delete;
    NativeVfsNameResolutionAcquired& operator=(const NativeVfsNameResolutionAcquired&) = delete;
    NativeVfsNameResolutionPhase phase() const noexcept;
    std::uint32_t active_call_site() const noexcept;
    std::uint32_t failure_site() const noexcept;
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
    friend bool resolve_native_vfs_existing_name_00bdf4c0(void*, void*,
        NativeVfsNameResolutionContext&, NativeVfsNameResolutionAcquired&);
};

// Full BDF4C0: ECX captured actual manager, mutable8h name stack, RET4, AL.
// Normalize caller; copy normalized name; execute native BDDC80 ordering;
// successful lookup calls BDEB40. False may still leave the caller mutated.
// Original numeric profiles remain on actual owners. Unknown current methods
// are explicit source boundaries, never a successful or false default.
bool resolve_native_vfs_existing_name_00bdf4c0(void* actual_manager,
    void* actual_mutable_name, NativeVfsNameResolutionContext&,
    NativeVfsNameResolutionAcquired&);

// BDEB40: ECX manager; original/resolved headers stack; RET8. Tests current
// 0109CEE8 and manager+79; constructs and destroys the native SRCH builder.
// There is no native sink/output call in this body.
void log_native_vfs_resolved_name_00bdeb40(void* actual_manager,
    const void* normalized_original, const void* resolved,
    NativeVfsOpenLoggingContext&);
} // namespace bsp
