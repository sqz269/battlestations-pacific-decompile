#pragma once

#include <cstdint>
#include <memory>

namespace bsp {
struct NativeVfsNameResolutionContext;
struct NativeVfsMountRegistrationContext;
struct NativeVfsUnmountNameContext;

// All contexts must borrow the same actual VFS publication and string/pool
// domain. The registry is the caller's actual1Ch owner, not a publication copy.
struct NativePakRegistryBlockContext {
    void* volatile& actual_vfs_publication_0109ceec;
    NativeVfsNameResolutionContext& resolution;
    NativeVfsMountRegistrationContext& mounting;
    NativeVfsUnmountNameContext& unmounting;
    const char* actual_dot_00ce3a70;
};

enum class NativePakRegistryBlockPhase {
    fresh, naming, resolving, copying_device_name, selecting_device,
    constructing_dot, mounting, appending, removing_entries, unmounting,
    releasing_dot, updating_count, releasing_device_name, recomputing,
    releasing_name, complete, failed
};

// Publish before invoking either callback. This immovable frame owns stable
// mpak/device/dot headers, nested resolver/unmount acquired frames and captured
// call/cleanup values. On failure retain it and all borrowed context/registry/
// original-name storage; no replay or guessed cleanup is supplied. Destruction
// of an active/failed invocation terminates. This retains OUTER storage only:
// BDD850/BE1740 and other dependencies retain their existing internal temporary
// and cleanup limits; see the doc's exact escaping-call-site table.
class NativePakRegistryBlockInvocation final {
public:
    NativePakRegistryBlockInvocation();
    ~NativePakRegistryBlockInvocation();
    NativePakRegistryBlockInvocation(const NativePakRegistryBlockInvocation&) = delete;
    NativePakRegistryBlockInvocation& operator=(const NativePakRegistryBlockInvocation&) = delete;
    NativePakRegistryBlockPhase phase() const noexcept;
    std::uint32_t active_call_site() const noexcept;
    std::uint32_t failure_site() const noexcept;
    const void* actual_mpak_header() const noexcept;
    const void* actual_device_header() const noexcept;
    const void* actual_dot_header() const noexcept;
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
    friend void enter_native_pak_registry_block_00bb5770(void*, const void*,
        NativePakRegistryBlockContext&, NativePakRegistryBlockInvocation&);
    friend void leave_native_pak_registry_block_00bb5910(void*, const void*,
        NativePakRegistryBlockContext&, NativePakRegistryBlockInvocation&);
};

// Complete ordinary BB5770..BB590A[411]. Native ECX captured registry,
// stack name, RET4. Clear CURRENT VFS+20, resolve mpak header, then on true:
// copy device name, select device, increment/capture ordinal, mount under '.',
// append returned raw provider (including null), release dot, increment CURRENT
// VFS count, release device copy. Recompute CURRENT active then release mpak.
void enter_native_pak_registry_block_00bb5770(void* actual_registry,
    const void* actual_name, NativePakRegistryBlockContext&,
    NativePakRegistryBlockInvocation&);

// Complete ordinary BB5910..BB5AB0[417]. Same native ABI. Ignore resolver AL;
// erase ALL equal-length/case-insensitive provider-name matches from the actual
// raw vector without releasing values, then unmount once under '.'. Release dot
// before testing the saved result; only true decrements CURRENT VFS count then
// registry ordinal. Always recompute CURRENT active before releasing mpak.
void leave_native_pak_registry_block_00bb5910(void* actual_registry,
    const void* actual_name, NativePakRegistryBlockContext&,
    NativePakRegistryBlockInvocation&);

// Explicit-service MSVC Win32 APIs, not native stack/FH3/SEH replacements.
// No automatic rollback, origin-manager capture or provider retention. Outer
// normal cleanup preserves throwing getter/return calls; native EH and nested
// dependency failures are separately qualified. No production/gameplay proof.
} // namespace bsp
