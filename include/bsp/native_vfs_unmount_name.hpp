#pragma once

#include <cstdint>
#include <memory>

namespace bsp {
class NativeAdoptedSubstreamDispatch;
struct NativePathCanonicalizerContext;
struct SingletonLifetimeCallbacks;

struct NativeVfsUnmountNameContext {
    NativePathCanonicalizerContext& canonicalizer;
    const SingletonLifetimeCallbacks& invalid_parameters;
    NativeAdoptedSubstreamDispatch& providers;
    // Actual CE7898 storage (verified "/"). No preflight provider dispatch.
    const char* trim_set_00ce7898;
};

enum class NativeVfsUnmountNamePhase {
    fresh, canonicalizing, trimming, searching, releasing_provider, erasing,
    releasing_name, complete, failed
};

// Publish before invocation. The canonical header, actual owner/node iterator,
// and captured callback/cleanup storage keep stable addresses. On any escaping
// source exception this frame, context, manager and borrowed inputs must remain
// alive: no replay or guessed native unwind is offered. Destroying an active or
// failed frame terminates, as with NativeFileStoreRequestAcquired.
class NativeVfsUnmountNameAcquired final {
public:
    NativeVfsUnmountNameAcquired();
    ~NativeVfsUnmountNameAcquired();
    NativeVfsUnmountNameAcquired(const NativeVfsUnmountNameAcquired&) = delete;
    NativeVfsUnmountNameAcquired& operator=(const NativeVfsUnmountNameAcquired&) = delete;
    NativeVfsUnmountNamePhase phase() const noexcept;
    std::uint32_t active_call_site() const noexcept;
    std::uint32_t failure_site() const noexcept;
    const void* actual_normalized_header() const noexcept;
    const void* actual_iterator() const noexcept;
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
    friend bool unmount_native_vfs_name_00be0750(void*, const void*, const void*,
        NativeVfsUnmountNameContext&, NativeVfsUnmountNameAcquired&);
};

// Complete ordinary BE0750..BE0915 (454 bytes). Native ECX manager, stacked
// system-name/mount-prefix actual8h headers, AL success, RET8. Canonicalize and
// right-trim the prefix, scan actual manager+3C, and remove only the first match
// of equal-length, CRT case-insensitive provider+8 name and node+10 mount key.
// InterlockedDecrement captured provider+4; only at zero capture CURRENT table
// and virtual0 and call providers.source_zero_reference; then erase BE0080 into
// the SAME local iterator using the captured owner/node. No provider retain.
bool unmount_native_vfs_name_00be0750(void* actual_manager,
    const void* actual_system_name, const void* actual_mount_prefix,
    NativeVfsUnmountNameContext&, NativeVfsUnmountNameAcquired&);

// New explicit-service Win32 C++ API, not original stack/SEH/FH3 ABI. Native
// CC6728/E00D7C state0 -> CC6720 destroys the canonical header; unknown nested
// failures instead retain the source frame. Ordinary final cleanup permits a
// throwing pool getter. Existing trim/erase NativeStringStorage releases remain
// noexcept, excluding native throwing-getter cleanup within those dependencies.
// Current CRT locale, valid actual storage and synchronous callback mutation
// are required; arbitrary stack aliases/concurrency and gameplay are unproved.
} // namespace bsp
