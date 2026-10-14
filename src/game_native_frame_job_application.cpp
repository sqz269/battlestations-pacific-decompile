#include "bsp/game_native_frame_job_application.hpp"
#include "bsp/game_native_readonly_data.hpp"
#include "bsp/native_render_job_publication.hpp"
#include "bsp/native_singleton_destruction.hpp"
#include <exception>
#include <optional>
#include <stdexcept>

namespace bsp::game {
namespace {
const volatile std::uint32_t* table(GameNativeReadOnlyData& data, std::uintptr_t address,
    std::size_t words) {
    return static_cast<const volatile std::uint32_t*>(data.data_at(address, words * 4));
}
NativeFixedStepJobTables descriptor_tables(GameNativeReadOnlyData& data) {
    return {table(data, 0x00d0deb4, 1), table(data, 0x00d0dec4, 1),
        table(data, 0x00d0deac, 1), table(data, 0x00d0debc, 1)};
}
}
struct GameNativeFrameJobApplication::Impl {
    // Explicit Source callable-entry binding, not an invented native game global.
    // Win32 still receives the original actual pool argument. The one canonical
    // process graph is installed before construction can create suspended workers
    // and removed only after original teardown has joined every actual worker.
    static std::atomic<Impl*> worker_binding;
    NativeRenderJobPublicationContext& publication;
    NativeSingletonDeletionBindings& deletion;
    NativeFixedStepJobDescriptorsActual descriptors;
    std::atomic<bool> interrupted{false};
    NativeFixedStepJobDispatch fixed_dispatch;
    struct ObservedDispatch final : NativeFrameJobDispatch {
        Impl& application;
        explicit ObservedDispatch(Impl& owner) : application(owner) {}
        void execute_current_00(void* owner, std::uint32_t argument) override {
            try { application.fixed_dispatch.execute_current_00(owner, argument); }
            catch (...) { application.interrupted.store(true); throw; }
        }
    } observed_dispatch;
    NativeFrameJobExecution execution;
    NativeFrameJobLifetimeBindings lifetime;
    std::optional<NativeFixedStepJobWaves> waves;
    void* groups{};
    void* head{};
    void* tail{};
    const volatile std::uint8_t* run_pass{};
    bool retired{};

    Impl(GameNativeReadOnlyData& data, NativeRenderJobPublicationContext& context,
        NativeSingletonDeletionBindings& bindings, RandomThreads& random,
        std::atomic<std::int32_t>& scope, NativeFixedStepJobDescriptorsActual descriptor_cells,
        NativeFixedStepTickBindings& ticks, NativeFrameJobDispatch& remaining)
        : publication(context), deletion(bindings), descriptors(descriptor_cells),
          fixed_dispatch(*static_cast<const volatile float*>(data.data_at(0x00d0de84, 4)),
              descriptor_tables(data), ticks, remaining), observed_dispatch(*this),
          execution(observed_dispatch, random, table(data, 0x00d6821c, 4),
              table(data, 0x00d68650, 6), table(data, 0x00ce7554, 6)),
          lifetime(execution, scope, &worker_entry) {
        if (!descriptors.lifetime_01090aa0.uses_actual_storage()
            || !descriptors.lifetime_01090aa0.borrows_same_domain(context.actual_manager_01090aa0))
            throw std::invalid_argument("Frame jobs require the same actual application AA0 domain");
        if (context.actual_frame_0109cf08 || context.frame_lifetime
            || descriptors.wave1_00f878d4 || descriptors.wave2_00f878dc
            || descriptors.wave3_00f878d0 || descriptors.interpolation_00f878d8
            || (deletion.render_jobs && deletion.render_jobs != &context) || deletion.fixed_step_jobs)
            throw std::logic_error("Cannot replace or adopt an existing frame-job execution graph");
        Impl* expected = nullptr;
        if (!worker_binding.compare_exchange_strong(expected, this))
            throw std::logic_error("The canonical process frame-job worker binding already exists");
        // No further throwing operations after entry publication.
        context.frame_lifetime = &lifetime;
        deletion.render_jobs = &context;
        deletion.fixed_step_jobs = &descriptors;
    }
    static unsigned long __stdcall worker_entry(void* raw_pool) {
        auto* const application = worker_binding.load();
        if (!application) throw std::logic_error("Missing canonical frame-job worker binding");
        auto* const current = application->publication.actual_frame_0109cf08;
        if (!current || &current->pool_04 != raw_pool) {
            application->interrupted.store(true);
            throw std::logic_error("Frame-job worker argument is not the actual canonical pool");
        }
        // The original worker body alone owns random register/unregister,
        // wait/reset/claim/signal, failure unwind and return-zero behavior.
        try { return native_frame_job_thread_entry_00be2ba0(current->pool_04, application->execution); }
        catch (...) { application->interrupted.store(true); throw; }
    }
    void require_live() const {
        if (retired || interrupted.load() || publication.frame_lifetime != &lifetime
            || deletion.render_jobs != &publication || deletion.fixed_step_jobs != &descriptors)
            throw std::logic_error("Canonical frame-job graph is retired, interrupted or retargeted");
    }
    NativeFrameJobOwnerStorage& owner() {
        require_live();
        try {
            auto* const result = get_native_frame_jobs_actual_004c1130(publication);
            if (!result) throw std::logic_error("Actual native frame-job factory returned null");
            return *result;
        } catch (...) { interrupted.store(true); throw; }
    }
    void retire() {
        if (retired) return;
        require_live();
        if (publication.actual_manager_01090aa0 || publication.actual_frame_0109cf08
            || descriptors.wave1_00f878d4 || descriptors.wave2_00f878dc
            || descriptors.wave3_00f878d0 || descriptors.interpolation_00f878d8)
            throw std::logic_error("Frame-job bindings must survive the complete canonical manager drain");
        Impl* expected = this;
        if (!worker_binding.compare_exchange_strong(expected, nullptr))
            throw std::logic_error("Canonical frame-job worker entry was replaced");
        publication.frame_lifetime = nullptr;
        deletion.fixed_step_jobs = nullptr;
        // The context also serves preparation jobs. Its storage remains owned
        // by the parent; retain that existing resolver reference after retirement.
        waves.reset();
        retired = true;
    }
};
std::atomic<GameNativeFrameJobApplication::Impl*> GameNativeFrameJobApplication::Impl::worker_binding{};

GameNativeFrameJobApplication::GameNativeFrameJobApplication(GameNativeReadOnlyData& data,
    NativeRenderJobPublicationContext& publication, NativeSingletonDeletionBindings& deletion,
    RandomThreads& random, std::atomic<std::int32_t>& scope,
    NativeFixedStepJobDescriptorsActual descriptors, NativeFixedStepTickBindings& ticks,
    NativeFrameJobDispatch& remaining)
    : impl_(std::make_unique<Impl>(data, publication, deletion, random, scope, descriptors, ticks, remaining)) {}
GameNativeFrameJobApplication::~GameNativeFrameJobApplication() noexcept {
    if (!impl_) return;
    try { impl_->retire(); }
    catch (...) { std::terminate(); }
}
NativeFrameJobOwnerStorage& GameNativeFrameJobApplication::frame_owner() { return impl_->owner(); }
void GameNativeFrameJobApplication::enqueue_actual(void* owner, std::uint32_t argument) {
    auto& p = *impl_;
    auto& pool = p.owner().pool_04;
    try {
        p.execution.require_enqueue_virtual_04(pool);
        enqueue_native_frame_job_00be3020(pool, owner, argument);
    } catch (...) { p.interrupted.store(true); throw; }
}
void GameNativeFrameJobApplication::dispatch_actual(std::uint8_t run_pass) {
    auto& p = *impl_;
    auto& pool = p.owner().pool_04;
    try {
        p.execution.require_dispatch_virtual_08(pool);
        dispatch_native_frame_jobs_00be3150(pool, run_pass, p.lifetime.scope, p.execution);
    } catch (...) { p.interrupted.store(true); throw; }
}
bool GameNativeFrameJobApplication::dispatching_current() {
    auto& p = *impl_;
    auto& pool = p.owner().pool_04;
    try { return p.execution.is_dispatching_current_10(pool); }
    catch (...) { p.interrupted.store(true); throw; }
}
NativeFixedStepJobWaves& GameNativeFrameJobApplication::bind_actual_fixed_step_groups(
    void* groups, void* head, void* tail, const volatile std::uint8_t& run_pass) {
    auto& p = *impl_;
    p.require_live();
    if (p.waves) {
        if (p.groups != groups || p.head != head || p.tail != tail || p.run_pass != &run_pass)
            throw std::logic_error("Cannot replace canonical fixed-step group/driver storage");
        return *p.waves;
    }
    if (!groups || !head || !tail) throw std::invalid_argument("Actual raw fixed-step group backing is required");
    (void)p.owner();
    p.waves.emplace(groups, head, tail, run_pass, p.publication, p.descriptors, p.interrupted);
    p.groups = groups; p.head = head; p.tail = tail; p.run_pass = &run_pass;
    return *p.waves;
}
bool GameNativeFrameJobApplication::requires_process_retention() const noexcept {
    const auto& p = *impl_;
    if (p.retired) return false;
    if (p.interrupted.load() || p.publication.frame_lifetime != &p.lifetime
        || p.deletion.render_jobs != &p.publication || p.deletion.fixed_step_jobs != &p.descriptors)
        return true;
    auto* const frame = p.publication.actual_frame_0109cf08;
    // Registered descriptors pop before the frame owner. A queued/active pool
    // cannot safely enter that normal drain, even if no callback threw yet.
    return frame && (frame->pool_04.remaining_20.load(std::memory_order_relaxed) > 0
        || frame->pool_04.dispatch_active_18.load(std::memory_order_relaxed) > 0);
}
void GameNativeFrameJobApplication::retire_after_manager_drain() { impl_->retire(); }
} // namespace bsp::game
