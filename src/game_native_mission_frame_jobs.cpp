#include "bsp/game_native_mission_frame_jobs.hpp"
#include "bsp/game_native_readonly_data.hpp"
#include "bsp/native_frame_job_lifetime.hpp"
#include "bsp/native_render_batch_preparation_actual.hpp"
#include "bsp/native_render_job_publication.hpp"
#include "bsp/native_singleton_destruction.hpp"
#include <atomic>
#include <exception>
#include <stdexcept>

namespace bsp::game {
namespace {
const volatile std::uint32_t* table(GameNativeReadOnlyData& data,
    std::uintptr_t address, std::size_t words) {
    return static_cast<const volatile std::uint32_t*>(data.data_at(address, words * 4));
}
}

struct GameNativeMissionFrameJobs::Impl {
    // Source entry metadata only. Win32 receives the actual secondary pool,
    // and the original worker body owns random registration and job ordering.
    static std::atomic<Impl*> worker_binding;
    NativeSingletonDeletionBindings& deletion;
    NativeFrameJobOwnerStorage* volatile frame_0109cf08{};
    NativeRenderPreparationJobStorage* volatile preparation_00f8d444{};
    std::atomic<std::int32_t> scope_0109dbe4{0};
    std::atomic<bool> interrupted{false};
    std::atomic<std::uint32_t> entered_workers{0};

    struct UnknownJob final : NativeFrameJobDispatch {
        void execute_current_00(void*, std::uint32_t) override {
            throw std::logic_error("Mission frame jobs require a bound current native job profile");
        }
    } unknown_job;
    NativeRenderPreparationJobDispatch preparation_dispatch;
    NativeFrameJobExecution execution;
    NativeFrameJobLifetimeBindings lifetime;
    NativeRenderJobPublicationContext publication;
    bool retired{};

    Impl(GameNativeReadOnlyData& data, NativeRenderBatchPreparationContext& queue,
        NativeSingletonDeletionBindings& bindings, RandomThreads& random)
        : deletion(bindings),
          preparation_dispatch(queue, table(data, 0x00d5e160, 1), unknown_job),
          execution(preparation_dispatch, random, table(data, 0x00d6821c, 4),
              table(data, 0x00d68650, 6), table(data, 0x00ce7554, 6)),
          lifetime(execution, scope_0109dbe4, &worker_entry),
          publication{queue.actual_manager_01090aa0, frame_0109cf08,
              preparation_00f8d444, &lifetime} {
        if (bindings.render_jobs)
            throw std::logic_error("Cannot replace an existing canonical render-job publication binding");
        Impl* expected = nullptr;
        if (!worker_binding.compare_exchange_strong(expected, this))
            throw std::logic_error("The canonical mission frame-job worker binding already exists");
        // No further throwing operations after publishing the entry binding.
        deletion.render_jobs = &publication;
    }

    static unsigned long __stdcall worker_entry(void* raw_pool) {
        auto* const application = worker_binding.load();
        if (!application)
            throw std::logic_error("Missing canonical mission frame-job worker binding");
        application->entered_workers.fetch_add(1, std::memory_order_relaxed);
        try {
            auto* const current = application->publication.actual_frame_0109cf08;
            if (!current || &current->pool_04 != raw_pool)
                throw std::logic_error("Mission frame-job argument is not the actual canonical pool");
            const auto result = native_frame_job_thread_entry_00be2ba0(
                current->pool_04, application->execution);
            application->entered_workers.fetch_sub(1, std::memory_order_relaxed);
            return result;
        } catch (...) {
            application->interrupted.store(true);
            application->entered_workers.fetch_sub(1, std::memory_order_relaxed);
            throw;
        }
    }

    bool same_bindings() const noexcept {
        return publication.frame_lifetime == &lifetime
            && deletion.render_jobs == &publication && worker_binding.load() == this;
    }
    void require_live() const {
        if (retired || interrupted.load() || !same_bindings())
            throw std::logic_error("Canonical mission frame jobs are retired, interrupted or retargeted");
    }
    bool retain() const noexcept {
        if (retired) return false;
        if (interrupted.load() || !same_bindings()
            || scope_0109dbe4.load(std::memory_order_relaxed) != 0)
            return true;
        auto* const frame = publication.actual_frame_0109cf08;
        // Idle, started workers may be joined by the native manager drain.
        // Queued/active work must instead retain all of its borrowed providers.
        return frame && (frame->pool_04.remaining_20.load(std::memory_order_relaxed) != 0
            || frame->pool_04.dispatch_active_18.load(std::memory_order_relaxed) != 0);
    }
    void retire() {
        if (retired) return;
        require_live();
        if (publication.actual_manager_01090aa0 || publication.actual_frame_0109cf08
            || publication.actual_preparation_00f8d444
            || scope_0109dbe4.load(std::memory_order_relaxed) != 0
            || entered_workers.load(std::memory_order_relaxed) != 0)
            throw std::logic_error("Mission frame-job bindings must survive the canonical manager drain and worker join");
        Impl* expected = this;
        if (!worker_binding.compare_exchange_strong(expected, nullptr))
            throw std::logic_error("Canonical mission frame-job worker entry was replaced");
        publication.frame_lifetime = nullptr;
        deletion.render_jobs = nullptr;
        retired = true;
    }
};

std::atomic<GameNativeMissionFrameJobs::Impl*>
    GameNativeMissionFrameJobs::Impl::worker_binding{};

GameNativeMissionFrameJobs::GameNativeMissionFrameJobs(GameNativeReadOnlyData& data,
    NativeRenderBatchPreparationContext& queue, NativeSingletonDeletionBindings& deletion,
    RandomThreads& random)
    : impl_(std::make_unique<Impl>(data, queue, deletion, random)) {}

GameNativeMissionFrameJobs::~GameNativeMissionFrameJobs() noexcept {
    if (!impl_) return;
    try { impl_->retire(); }
    catch (...) { std::terminate(); }
}

NativeRenderJobPublicationContext& GameNativeMissionFrameJobs::publication() {
    impl_->require_live();
    return impl_->publication;
}
NativeRenderPreparationJobDispatch& GameNativeMissionFrameJobs::preparation_dispatch() {
    impl_->require_live();
    return impl_->preparation_dispatch;
}
void GameNativeMissionFrameJobs::observe_interrupted() noexcept {
    impl_->interrupted.store(true);
}
bool GameNativeMissionFrameJobs::requires_process_retention() const noexcept {
    return impl_->retain();
}
void GameNativeMissionFrameJobs::after_native_drain() {
    impl_->retire();
}
} // namespace bsp::game
