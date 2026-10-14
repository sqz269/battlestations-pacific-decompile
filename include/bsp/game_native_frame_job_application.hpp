#pragma once

#include "bsp/native_fixed_step_job_waves.hpp"
#include <memory>

namespace bsp {
struct NativeRenderJobPublicationContext;
struct NativeSingletonDeletionBindings;
}
namespace bsp::game {
class GameNativeReadOnlyData;

// Stable application execution/worker binding for the canonical raw frame owner.
// Borrow the SAME existing AA0/CF08/preparation context, deletion map, startup
// RandomThreads, actual DBE4 counter, four descriptor cells and complete current
// receiver/remaining-job dispatchers. No owner is allocated by construction.
// There is one process worker-entry binding; no pool, thread argument, event,
// queue, random registry, table metadata or processor count is substituted.
class GameNativeFrameJobApplication final {
public:
    GameNativeFrameJobApplication(GameNativeReadOnlyData&,
        NativeRenderJobPublicationContext&, NativeSingletonDeletionBindings&,
        RandomThreads& existing_startup_random, std::atomic<std::int32_t>& actual_scope_0109dbe4,
        NativeFixedStepJobDescriptorsActual, NativeFixedStepTickBindings& actual_ticks,
        NativeFrameJobDispatch& existing_remaining_jobs);
    ~GameNativeFrameJobApplication() noexcept;
    GameNativeFrameJobApplication(const GameNativeFrameJobApplication&) = delete;
    GameNativeFrameJobApplication& operator=(const GameNativeFrameJobApplication&) = delete;
    GameNativeFrameJobApplication(GameNativeFrameJobApplication&&) = delete;
    GameNativeFrameJobApplication& operator=(GameNativeFrameJobApplication&&) = delete;

    // Native 4C1130 uses the actual manager/publication and original processor
    // getter/constructor. Failure preserves original partial state and marks
    // process retention; there is no replay, fallback pool or automatic cleanup.
    NativeFrameJobOwnerStorage& frame_owner();
    void enqueue_actual(void* actual_job_owner, std::uint32_t raw_argument);
    void dispatch_actual(std::uint8_t actual_run_pass);
    bool dispatching_current(); // virtual10 is signed active>0, not thread identity

    // Bind only genuine initialized groups/pending sentinels/current driver byte.
    // The returned raw service can attach to GameFixedStepHost. No nodes are
    // produced here; once bound, replacement group/storage identities are refused.
    NativeFixedStepJobWaves& bind_actual_fixed_step_groups(void* actual_groups_00f876c0,
        void* actual_pending_head_00e0b6d0, void* actual_pending_tail_00e0b704,
        const volatile std::uint8_t& actual_run_pass_00e0b6ce);

    bool requires_process_retention() const noexcept;
    // The application MUST inspect retention before its raw manager drain.
    // Retire only AFTER that same canonical drain has returned and cleared AA0,
    // CF08 and all four descriptors. Original scalar cleanup stops/joins workers.
    // This retires Source bindings, never drains/deletes native owners itself.
    // Destroying an active/interrupted graph terminates instead of freeing live
    // worker dependencies. Retain it until OS exit when interrupted.
    void retire_after_manager_drain();
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
} // namespace bsp::game
