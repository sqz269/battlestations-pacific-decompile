#pragma once

#include <memory>

namespace bsp {
class RandomThreads;
class NativeRenderPreparationJobDispatch;
struct NativeRenderBatchPreparationContext;
struct NativeRenderJobPublicationContext;
struct NativeSingletonDeletionBindings;
}

namespace bsp::game {
class GameNativeReadOnlyData;

// The renderer application's sole Source CF08/F8D444/DBE4 storage and worker
// binding. Borrow the SAME queue getter/raw manager, deletion map and already
// initialized startup RandomThreads. Construction creates no native owners.
// The direct execution dispatcher is the genuine D5E160:B1BF70 preparation
// dispatcher; unknown jobs fail rather than using a serial or empty substitute.
class GameNativeMissionFrameJobs final {
public:
    GameNativeMissionFrameJobs(GameNativeReadOnlyData&,
        NativeRenderBatchPreparationContext& same_queue_getter,
        NativeSingletonDeletionBindings& same_deletion_map,
        RandomThreads& existing_startup_random);
    ~GameNativeMissionFrameJobs() noexcept;
    GameNativeMissionFrameJobs(const GameNativeMissionFrameJobs&) = delete;
    GameNativeMissionFrameJobs& operator=(const GameNativeMissionFrameJobs&) = delete;
    GameNativeMissionFrameJobs(GameNativeMissionFrameJobs&&) = delete;
    GameNativeMissionFrameJobs& operator=(GameNativeMissionFrameJobs&&) = delete;

    // Stable borrowed identities for NativeRenderCommandExecutionContext.
    // These getters do not run 4C1130/B0FFB0, enqueue, dispatch or reset cells.
    NativeRenderJobPublicationContext& publication();
    NativeRenderPreparationJobDispatch& preparation_dispatch();

    // B1D950 invokes native factories/dispatch directly. Its caller MUST mark
    // any escaped exception, including construction before CF08 publication.
    // Native partial construction/execution has no invented cleanup or retry.
    void observe_interrupted() noexcept;
    bool requires_process_retention() const noexcept;

    // Call after the SAME raw manager drain returns, while all borrowed data
    // and the startup random registry are still live. Native deletion owns
    // descriptor retirement and worker stop/join. This checks their null/join
    // result and retires only Source bindings; it never drains native owners.
    void after_native_drain();

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
} // namespace bsp::game
