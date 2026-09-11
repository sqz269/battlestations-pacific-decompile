#include "bsp/native_frame_job_execution.hpp"
#include <cstring>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native frame-job execution requires MSVC Win32.
#endif

namespace bsp {
namespace {
std::int32_t signed_word(std::uint32_t word) noexcept {
    std::int32_t value;
    std::memcpy(&value, &word, 4);
    return value;
}
std::int32_t decrement(std::atomic<std::int32_t>& value) noexcept {
    return signed_word(static_cast<std::uint32_t>(value.fetch_sub(1, std::memory_order_seq_cst)) - 1u);
}
std::int32_t increment(std::atomic<std::int32_t>& value) noexcept {
    return signed_word(static_cast<std::uint32_t>(value.fetch_add(1, std::memory_order_seq_cst)) + 1u);
}
NativeFrameJobSlot& slot(NativeFrameJobPoolStorage& pool, std::int32_t index) noexcept {
    const auto address = reinterpret_cast<std::uintptr_t>(&pool) + 0x24u
        + static_cast<std::uint32_t>(index) * 8u;
    return *reinterpret_cast<NativeFrameJobSlot*>(address);
}
void claim_and_execute(NativeFrameJobPoolStorage& pool, NativeFrameJobExecution& execution) {
    for (auto index = decrement(pool.remaining_20); index >= 0; index = decrement(pool.remaining_20)) {
        auto& current = slot(pool, index);
        void* const owner = current.owner_00;
        const auto argument = current.argument_04;
        execution.execute(owner, argument);
        current.owner_00 = nullptr;
        current.argument_04 = 0;
    }
    (void)increment(pool.remaining_20);
}
class RandomThreadUnwind final {
public:
    explicit RandomThreadUnwind(RandomThreads& value) noexcept : random_(value) {}
    ~RandomThreadUnwind() noexcept { if (armed) random_.unregister_current_00bd3050(); }
    bool armed{true};
private:
    RandomThreads& random_;
};
}

NativeFrameJobExecution::NativeFrameJobExecution(NativeFrameJobDispatch& jobs,
    RandomThreads& random, const volatile std::uint32_t* events,
    const volatile std::uint32_t* base, const volatile std::uint32_t* frame)
    : jobs_(jobs), random_(random), event_table_(events), base_table_(base), frame_table_(frame) {
    if (!events || !base || !frame || events[0] != 0x00bd19b0u
        || events[1] != 0x00bd1910u || events[2] != 0x00bd17c0u || events[3] != 0x00bd1960u
        || base[0] != 0x00be31e0u || frame[0] != 0x004bfac0u)
        throw std::invalid_argument("Frame job execution requires the actual event and pool tables");
    for (const auto* table : {base, frame}) {
        if (table[1] != 0x00be3020u || table[2] != 0x00be3150u || table[3] != 0x00be2ea0u
            || table[4] != 0x004bfab0u || table[5] != 0x004bfaa0u)
            throw std::invalid_argument("Unsupported current frame job pool profile");
    }
}
void NativeFrameJobExecution::execute(void* owner, std::uint32_t argument) {
    jobs_.execute_current_00(owner, argument);
}
void NativeFrameJobExecution::require_event(const NativeEventOwnerStorage& owner,
    std::size_t slot_index, std::uint32_t expected) const {
    if (owner.table_00 != native_event_concrete_table_00d6821c || event_table_[slot_index] != expected)
        throw std::invalid_argument("Missing current native frame job event binding");
}
void NativeFrameJobExecution::signal(NativeEventOwnerStorage* owner) {
    require_event(*owner, 1, 0x00bd1910u);
    (void)signal_native_event_owner_00bd1910(owner);
}
void NativeFrameJobExecution::wait(NativeEventOwnerStorage* owner) {
    require_event(*owner, 2, 0x00bd17c0u);
    (void)wait_native_event_owner_00bd17c0(owner);
}
void NativeFrameJobExecution::reset(NativeEventOwnerStorage* owner) {
    require_event(*owner, 3, 0x00bd1960u);
    (void)reset_native_event_owner_00bd1960(owner);
}
void NativeFrameJobExecution::require_worker_virtual_0c(const NativeFrameJobPoolStorage& pool) const {
    const auto identity = pool.native_vtable_00;
    const auto* table = identity == 0x00d68650u ? base_table_
        : identity == 0x00ce7554u ? frame_table_ : nullptr;
    if (!table || table[3] != 0x00be2ea0u)
        throw std::invalid_argument("Missing current native frame job worker binding");
}

void frame_job_optional_callback_004bfaa0(std::uint32_t) noexcept {}
bool frame_job_dispatch_active_004bfab0(const NativeFrameJobPoolStorage& pool) noexcept {
    return pool.dispatch_active_18.load(std::memory_order_relaxed) > 0;
}
std::int32_t enter_frame_job_scope_00bf2ce0(std::atomic<std::int32_t>& counter) noexcept {
    return increment(counter);
}
std::int32_t leave_frame_job_scope_00bf2cf0(std::atomic<std::int32_t>& counter) noexcept {
    return decrement(counter);
}
void enqueue_native_frame_job_00be3020(NativeFrameJobPoolStorage& pool, void* job,
    std::uint32_t argument) noexcept {
    (void)pool.remaining_20.load(std::memory_order_relaxed);
    (void)pool.remaining_20.load(std::memory_order_relaxed);
    const auto index = pool.remaining_20.load(std::memory_order_relaxed);
    auto& current = slot(pool, index);
    current.owner_00 = job;
    current.argument_04 = argument;
    pool.remaining_20.store(signed_word(static_cast<std::uint32_t>(
        pool.remaining_20.load(std::memory_order_relaxed)) + 1u), std::memory_order_relaxed);
}
void drain_native_frame_jobs_00be2fa0(NativeFrameJobPoolStorage& pool, NativeFrameJobExecution& execution) {
    claim_and_execute(pool, execution);
    if (pool.worker_count_14 > 0) {
        for (std::int32_t index = 0; index < pool.worker_count_14; ++index)
            execution.wait(pool.done_events_10[index]);
    }
}
void wake_native_frame_job_workers_00be3110(NativeFrameJobPoolStorage& pool, NativeFrameJobExecution& execution) {
    for (std::int32_t index = 0; index < pool.worker_count_14; ++index) {
        execution.reset(pool.done_events_10[index]);
        execution.signal(pool.start_events_0c[index]);
    }
}
void dispatch_native_frame_jobs_00be3150(NativeFrameJobPoolStorage& pool, std::uint8_t use_workers,
    std::atomic<std::int32_t>& counter, NativeFrameJobExecution& execution) {
    (void)enter_frame_job_scope_00bf2ce0(counter);
    pool.dispatch_active_18.store(1, std::memory_order_relaxed);
    if (use_workers) {
        wake_native_frame_job_workers_00be3110(pool, execution);
        for (std::int32_t index = 0; index < pool.worker_count_14; ++index)
            (void)ResumeThread(pool.thread_handles_04[index]);
    } else {
        for (std::int32_t index = 0; index < pool.worker_count_14; ++index)
            execution.signal(pool.done_events_10[index]);
    }
    drain_native_frame_jobs_00be2fa0(pool, execution);
    pool.dispatch_active_18.store(0, std::memory_order_relaxed);
    (void)leave_frame_job_scope_00bf2cf0(counter);
}
void run_native_frame_job_worker_00be2ea0(NativeFrameJobPoolStorage& pool, NativeFrameJobExecution& execution) {
    auto& random = execution.random_threads();
    random.register_current_00bd2fe0();
    RandomThreadUnwind unwind(random);
    const auto thread_id = GetCurrentThreadId();
    std::int32_t worker = -1;
    for (std::int32_t index = 0; index < pool.worker_count_14; ++index) {
        if (pool.thread_ids_08[index] == thread_id) { worker = index; break; }
    }
    for (;;) {
        execution.wait(pool.start_events_0c[worker]);
        execution.reset(pool.start_events_0c[worker]);
        if (pool.stop_requested_1c.load(std::memory_order_relaxed) != 0) break;
        claim_and_execute(pool, execution);
        execution.signal(pool.done_events_10[worker]);
    }
    execution.signal(pool.done_events_10[worker]);
    unwind.armed = false;
    random.unregister_current_00bd3050();
}
std::uint32_t native_frame_job_thread_entry_00be2ba0(
    NativeFrameJobPoolStorage& pool, NativeFrameJobExecution& execution) {
    execution.require_worker_virtual_0c(pool);
    run_native_frame_job_worker_00be2ea0(pool, execution);
    return 0;
}

} // namespace bsp
