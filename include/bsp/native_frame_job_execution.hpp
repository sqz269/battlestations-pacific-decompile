#pragma once

#include "bsp/game_render_frame.hpp"
#include "bsp/native_event_owner.hpp"
#include "bsp/random_threads.hpp"
#include <array>
#include <atomic>
#include <cstddef>

namespace bsp {

struct NativeFrameJobSlot {
    void* volatile owner_00;
    volatile std::uint32_t argument_04;
};
// Actual 138A4h secondary pool at frame singleton primary+04. Construction,
// allocation, thread creation and lifetime are separate; no implicit reset.
// The three atomic DWORDs retain the physical layout. Plain native accesses
// use relaxed loads/stores; only the original Interlocked sites use RMWs.
struct NativeFrameJobPoolStorage {
    volatile std::uint32_t native_vtable_00;
    void** volatile thread_handles_04;
    std::uint32_t* volatile thread_ids_08;
    NativeEventOwnerStorage** volatile start_events_0c;
    NativeEventOwnerStorage** volatile done_events_10;
    volatile std::int32_t worker_count_14;
    std::atomic<std::int32_t> dispatch_active_18;
    std::atomic<std::int32_t> stop_requested_1c;
    std::atomic<std::int32_t> remaining_20;
    std::array<NativeFrameJobSlot, kFrameJobCapacity> slots_24;
};
static_assert(sizeof(NativeFrameJobSlot) == 8);
static_assert(sizeof(NativeFrameJobPoolStorage) == 0x138a4);
static_assert(offsetof(NativeFrameJobPoolStorage, remaining_20) == 0x20);
static_assert(offsetof(NativeFrameJobPoolStorage, slots_24) == 0x24);
static_assert(std::atomic<std::int32_t>::is_always_lock_free);

class NativeFrameJobDispatch {
public:
    virtual ~NativeFrameJobDispatch() = default;
    // REQUIRED exact owner's CURRENT virtual00 with the raw argument DWORD.
    // Use its established execution binding; no success fallback, substituted
    // job, implicit retain/release or shadow queue. The raw owner remains alive
    // through its callback; captured slot storage survives the subsequent clear.
    virtual void execute_current_00(void* actual_job_owner, std::uint32_t argument) = 0;
};

// Borrows the application's existing random-thread registry, actual native
// tables and job executor. No pool, thread, counter or event is owned here.
class NativeFrameJobExecution final {
public:
    NativeFrameJobExecution(NativeFrameJobDispatch&, RandomThreads&,
        const volatile std::uint32_t* actual_event_table_00d6821c,
        const volatile std::uint32_t* actual_base_table_00d68650,
        const volatile std::uint32_t* actual_frame_table_00ce7554);
    void execute(void*, std::uint32_t);
    void signal(NativeEventOwnerStorage*);
    void wait(NativeEventOwnerStorage*);
    void reset(NativeEventOwnerStorage*);
    void delete_event(NativeEventOwnerStorage*);
    void require_dispatch_virtual_08(const NativeFrameJobPoolStorage&) const;
    void require_worker_virtual_0c(const NativeFrameJobPoolStorage&) const;
    RandomThreads& random_threads() noexcept { return random_; }
private:
    void require_event(const NativeEventOwnerStorage&, std::size_t slot,
        std::uint32_t expected) const;
    NativeFrameJobDispatch& jobs_;
    RandomThreads& random_;
    const volatile std::uint32_t* event_table_;
    const volatile std::uint32_t* base_table_;
    const volatile std::uint32_t* frame_table_;
};

// 4BFAA0: native RET4 only, no consumed ECX/argument and no field writes.
void frame_job_optional_callback_004bfaa0(std::uint32_t ignored) noexcept;
// 4BFAB0: ECX secondary pool, AL=(SIGNED active18 > 0), RET. Upper EAX stale.
bool frame_job_dispatch_active_004bfab0(const NativeFrameJobPoolStorage&) noexcept;
// BF2CE0/BF2CF0: Interlocked increment/decrement of actual0109DBE4, RET,
// with resulting DWORD retained in EAX. No independent global is introduced.
std::int32_t enter_frame_job_scope_00bf2ce0(std::atomic<std::int32_t>& actual_0109dbe4) noexcept;
std::int32_t leave_frame_job_scope_00bf2cf0(std::atomic<std::int32_t>& actual_0109dbe4) noexcept;

// BE3020: ECX pool, stack actual job/raw argument, RET8. Three plain count
// reads precede the captured slot stores; increment CURRENT count afterward.
// No bounds checks, retain or synchronization with concurrent producers.
void enqueue_native_frame_job_00be3020(NativeFrameJobPoolStorage&, void* actual_job,
    std::uint32_t argument) noexcept;
// BE2FA0: ECX pool, RET. Atomic decrement claims LIFO slot, execute virtual00,
// clear captured slot AFTER callback, repeat; compensate negative claim once,
// then wait on each CURRENT done event using current worker count.
void drain_native_frame_jobs_00be2fa0(NativeFrameJobPoolStorage&, NativeFrameJobExecution&);
// BE3110: ECX pool, RET. Reset each current done event, then signal its current
// start event; reload worker count/arrays in the native order.
void wake_native_frame_job_workers_00be3110(NativeFrameJobPoolStorage&, NativeFrameJobExecution&);
// BE3150: ECX pool, stack low-byte use_workers, RET4. Increment shared scope,
// set active=1; nonzero wakes then ResumeThread, zero signals done events;
// caller drains/waits, clears active, decrements scope. There is NO local unwind
// rollback if job execution fails; do not add a finally guard for active/scope.
void dispatch_native_frame_jobs_00be3150(NativeFrameJobPoolStorage&, std::uint8_t use_workers,
    std::atomic<std::int32_t>& actual_0109dbe4, NativeFrameJobExecution&);

// BE2EA0: ECX pool, RET. Register real current random thread, resolve its actual
// Win32 ID in the pool, then wait/reset start, inspect stop, claim/execute/clear
// jobs, compensate negative claim and signal done. Stop signals done then
// unregisters. State0 unwind ONLY unregisters; no extra done signal or slot clear.
// The caller must be an actual listed worker, and events/arrays must stay alive.
void run_native_frame_job_worker_00be2ea0(NativeFrameJobPoolStorage&, NativeFrameJobExecution&);
// BE2BA0: Win32 stdcall thread entry (stack pool), EAX0, RET4. Invoke current
// virtual0C then return zero. This C++ interface receives its existing execution
// binding explicitly and validates the actual D68650/CE7554 worker slot.
std::uint32_t native_frame_job_thread_entry_00be2ba0(
    NativeFrameJobPoolStorage&, NativeFrameJobExecution&);

// Actual layout/new C++ ABI, not native integer-vtable execution. Owner lifetime
// is in native_frame_job_lifetime.hpp; complete job execution bindings remain
// required. Valid spans and coordinated producer/lifetime access are necessary.
} // namespace bsp
