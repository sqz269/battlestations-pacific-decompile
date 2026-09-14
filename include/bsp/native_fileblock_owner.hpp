#pragma once
#include <cstdint>
#include <memory>

namespace bsp {
class NativeStringStorage;
struct NativeStringRawPoolContext;
struct NativeVfsFileBlockScopeContext;
class NativeVfsFileBlockScopeAcquired;

struct NativeFileBlockOwnerContext {
    void* volatile& actual_vfs_publication_0109ceec;
    NativeStringStorage& strings;
    NativeStringRawPoolContext& raw_pool;
    NativeVfsFileBlockScopeContext& scopes;
};
enum class NativeFileBlockOwnerPhase {
    fresh, copying_name, identifying, entering, leaving, returning_name,
    destroying_base, freeing_owner, complete, failed
};

// Publish an immovable frame before each explicit construction/destruction.
// It retains captured fields and the nested BP scope invocation if a call
// escapes. Retain it and all actual caller storage on failure. Active/failed
// destruction terminates; there is no replay or automatic compensating exit.
// Native FH3 cleanup is recorded as evidence, not executed by this boundary.
class NativeFileBlockOwnerAcquired final {
public:
    NativeFileBlockOwnerAcquired();
    ~NativeFileBlockOwnerAcquired();
    NativeFileBlockOwnerAcquired(const NativeFileBlockOwnerAcquired&) = delete;
    NativeFileBlockOwnerAcquired& operator=(const NativeFileBlockOwnerAcquired&) = delete;
    NativeFileBlockOwnerPhase phase() const noexcept;
    std::uint32_t active_call_site() const noexcept;
    std::uint32_t failure_site() const noexcept;
    std::int32_t native_state_at_last_call() const noexcept;
    const NativeVfsFileBlockScopeAcquired& scope_invocation() const noexcept;
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
    friend void* construct_native_fileblock_00be0a30(void*, const void*, std::uint32_t,
        NativeFileBlockOwnerContext&, NativeFileBlockOwnerAcquired&);
    friend void destroy_native_fileblock_00bdcb30(void*, NativeFileBlockOwnerContext&, NativeFileBlockOwnerAcquired&);
    friend void* delete_native_fileblock_00bdebe0(void*, std::uint32_t, NativeFileBlockOwnerContext&, NativeFileBlockOwnerAcquired&);
};

// Actual1Ch owner: profile0, refcount4, zero DWORDs8/C/10, name14/18.
// BE0A30: ECX owner, stack(name,gate DWORD), EAX owner, RET8. Copy/identify
// the name, then read CURRENT0109CEEC and enter that actual manager.
void* construct_native_fileblock_00be0a30(void*, const void* actual_name,
    std::uint32_t gate, NativeFileBlockOwnerContext&, NativeFileBlockOwnerAcquired&);
// BDCB30: ECX owner, RET. StampD68494, read CURRENT0109CEEC, leave scope,
// capture name data before state0, raw getter/return, then stamp baseCEB130.
void destroy_native_fileblock_00bdcb30(void*, NativeFileBlockOwnerContext&, NativeFileBlockOwnerAcquired&);
// BDEBE0: ECX owner, stack flags, EAX captured owner, RET4. Free only bit0
// after normal destruction; neither this nor the base decrements references.
void* delete_native_fileblock_00bdebe0(void*, std::uint32_t flags,
    NativeFileBlockOwnerContext&, NativeFileBlockOwnerAcquired&);

enum class NativeLoadingJobFileBlockPhase {
    fresh, allocating, substring, constructing, publishing, returning_name, complete, failed
};
class NativeLoadingJobFileBlockAcquired final {
public:
    NativeLoadingJobFileBlockAcquired();
    ~NativeLoadingJobFileBlockAcquired();
    NativeLoadingJobFileBlockAcquired(const NativeLoadingJobFileBlockAcquired&) = delete;
    NativeLoadingJobFileBlockAcquired& operator=(const NativeLoadingJobFileBlockAcquired&) = delete;
    NativeLoadingJobFileBlockPhase phase() const noexcept;
    std::uint32_t active_call_site() const noexcept;
    std::uint32_t failure_site() const noexcept;
    const void* allocated_owner() const noexcept;
    const void* temporary_name_header() const noexcept;
    const NativeFileBlockOwnerAcquired& owner_invocation() const noexcept;
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
    friend void ensure_native_loading_job_fileblock_00504790(void*, NativeFileBlockOwnerContext&, NativeLoadingJobFileBlockAcquired&);
};
// 504790: ECX actual job, RET. Existing+20 or empty+14 skips. Allocate1Ch
// before substring(start13,wrapping storedLength-18), construct(temp,1), publish
// returned owner at job+20 before raw temporary return. No added length check.
void ensure_native_loading_job_fileblock_00504790(void* actual_job,
    NativeFileBlockOwnerContext&, NativeLoadingJobFileBlockAcquired&);

// Complete ordinary source bodies; new explicit-service ABI. Retention is not
// native unwind. Identifier/substring/string-storage and BP/BO internal-lifetime
// limits remain; native CRT/FH3/SEH identity and production binding are unproved.
} // namespace bsp
