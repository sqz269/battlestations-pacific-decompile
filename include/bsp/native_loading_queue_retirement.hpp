#pragma once
#include <cstdint>
#include <memory>
namespace bsp {
struct NativeFileBlockOwnerContext;
struct NativeFileStoreFactoryContext;
struct NativeFileStoreProviderLifetimeContext;
struct NativeStringRawPoolContext;
class NativeFileBlockOwnerAcquired;
struct NativeLoadingQueueRetireContext {
    NativeFileBlockOwnerContext& fileblocks;
    NativeFileStoreFactoryContext& factories;
    NativeFileStoreProviderLifetimeContext& providers;
    NativeStringRawPoolContext& raw_strings;
};
enum class NativeLoadingQueueRetirePhase {
    fresh, reading_front, deleting_fileblock, getting_factory, getting_provider,
    removing_file, destroying_job, freeing_job, shifting, complete, failed
};
// Retain this frame and borrowed queue/context storage on escaping calls.
// It owns the nested BQ FileBlock invocation; its native-unwind differences
// remain explicit. Active/failed destruction terminates, with no replay.
class NativeLoadingQueueRetireAcquired final {
public:
    NativeLoadingQueueRetireAcquired();
    ~NativeLoadingQueueRetireAcquired();
    NativeLoadingQueueRetireAcquired(const NativeLoadingQueueRetireAcquired&)=delete;
    NativeLoadingQueueRetireAcquired& operator=(const NativeLoadingQueueRetireAcquired&)=delete;
    NativeLoadingQueueRetirePhase phase() const noexcept;
    std::uint32_t failure_site() const noexcept;
    const void* initial_job() const noexcept;
    const void* job_selected_for_destruction() const noexcept;
    const void* captured_front_slot() const noexcept;
    const NativeFileBlockOwnerAcquired& fileblock_invocation() const noexcept;
private:
    struct Impl;std::unique_ptr<Impl> impl_;
    friend void retire_native_loading_queue_front_00506bf0(void*,NativeLoadingQueueRetireContext&,NativeLoadingQueueRetireAcquired&);
};
// Complete ordinary506BF0[131]. Native ECX loader20h, RET. No empty check.
// Read initial front job, delete current FileBlock+4(flags1), clear that job's
// +20 only after return. Reacquire current FileStore factory/provider and remove
// initial job's name. Then reload CURRENT array/front for job destruction/free,
// clear captured slot, shift using current array/count each iteration, and
// decrement current count. Capacity and vacated trailing slot are untouched.
// Current FileBlock table bytes must be readable; only reached BDEBE0 is bound.
// An unsupported reached target throws at506C07; original code is never called.
void retire_native_loading_queue_front_00506bf0(void* actual_loader,
    NativeLoadingQueueRetireContext&,NativeLoadingQueueRetireAcquired&);
// New source ABI; no worker thread, full queue update/drain or production hook.
} // namespace bsp
