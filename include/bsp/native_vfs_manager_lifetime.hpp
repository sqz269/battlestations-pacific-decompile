#pragma once

#include "bsp/sound_lifetime_access.hpp"
#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native VFS manager lifetime requires MSVC Win32.
#endif

namespace bsp {
class ActualNativeStringPoolStorage;
struct NativePathCanonicalizerContext;

// Required calls for the two native indirect sites. The manager reads each
// current vtable entry before passing its exact DWORD target, captured ECX owner
// and flags. A binding must invoke that target with the original RET4 contract;
// it must not re-read the publication, choose an owner by type or ignore a call.
// Actual logger and physical-provider deleting bodies are available separately.
// Other concrete provider targets still require their application binding.
class NativeVfsManagerVirtualCalls {
public:
    virtual ~NativeVfsManagerVirtualCalls() = default;
    virtual void invoke_log_virtual0_00be1fa1(std::uint32_t captured_target,
        void* captured_owner, std::uint32_t flags) = 0;
    virtual void invoke_provider_virtual4_00be1ffd(std::uint32_t captured_target,
        void* captured_owner, std::uint32_t flags) = 0;
};

// Borrow the application's actual publication cells, canonical string storage,
// existing lifetime manager and returning validation boundary. The canonicalizer
// uses that same application's services. Bind the six original C-string values
// in order D68464,D68454,D68444,D68438,D68414,D68400; all calls are observable.
struct NativeVfsManagerLifetimeContext {
    void* volatile& manager_0109ceec;
    void* volatile& file_access_log_0109cee8;
    SoundLifetimeAccess lifetime;
    ActualNativeStringPoolStorage& strings;
    NativePathCanonicalizerContext& canonicalizer;
    const char* const path_examples[6];
    const SingletonLifetimeCallbacks& invalid_parameters;
    NativeVfsManagerVirtualCalls& virtual_calls;
};

// BE1DC0[411]: ECX actual A0h owner, no stacked args, RET0, EAX owner.
// Complete raw construction, six actual allocation leaves, native sentinel
// stores and six BDB970 calls. Preserve opaque member words and padding.
// Source unwind uses the original ten-state cleanup chain and no extra rollback.
void* construct_native_vfs_manager_00be1dc0(void* actual_owner,
    NativeVfsManagerLifetimeContext&);

// BE1F60[643]: ECX owner, RET0. Delete captured current file log, walk mounts
// and invoke current provider virtual4(1), then all raw members in native order.
// Node+1C is not an ownership gate. Native raw post-free tails are included;
// Ghidra's stored function body can still end at BE2028.
void destroy_native_vfs_manager_00be1f60(void* actual_owner,
    NativeVfsManagerLifetimeContext&);

// BE25C0[30]: ECX owner, stacked flags, RET4, EAX original possibly freed owner.
// Always destruct, then free exactly when low bit0 is set.
void* delete_native_vfs_manager_00be25c0(void* actual_owner,
    std::uint32_t flags, NativeVfsManagerLifetimeContext&);

// These C++ interfaces preserve logical storage/call ordering, not original
// FH3/SEH/RTTI or binary ABI. Actual pooled release is noexcept; throwing native
// singleton-getter identity and simultaneous cleanup failure remain unvalidated.
// No game startup or gameplay proof follows from isolated owner fixtures.
} // namespace bsp
