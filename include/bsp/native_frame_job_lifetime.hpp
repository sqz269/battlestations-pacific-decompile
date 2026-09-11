#pragma once

#include "bsp/native_frame_job_execution.hpp"
#include "bsp/singleton_lifetime.hpp"

namespace bsp {

struct NativeFrameJobOwnerStorage {
    volatile std::uint32_t native_vtable_00;
    NativeFrameJobPoolStorage pool_04;
};
static_assert(sizeof(NativeFrameJobOwnerStorage) == 0x138a8);
static_assert(offsetof(NativeFrameJobOwnerStorage, pool_04) == 4);

using NativeFrameJobStartRoutine = unsigned long (__stdcall*)(void*);

// Borrows actual execution/scope and a REQUIRED application Win32 entry binding
// for BE2BA0. That entry receives the actual secondary pool address (never a
// substitute context) and calls native_frame_job_thread_entry_00be2ba0 with its
// existing execution binding. It must survive every created thread. No default
// entry, replacement random registry, hidden pool or successful fallback exists.
class NativeFrameJobLifetimeBindings final {
public:
    NativeFrameJobLifetimeBindings(NativeFrameJobExecution&,
        std::atomic<std::int32_t>& actual_scope_0109dbe4,
        NativeFrameJobStartRoutine bound_entry_00be2ba0);
    NativeFrameJobExecution& execution;
    std::atomic<std::int32_t>& scope;
    const NativeFrameJobStartRoutine entry;
};

// BE4800, cdecl/RET: _dupenv_s NUMBER_OF_PROCESSORS; nonzero errno returns1.
// Success returns the captured atol result after free, with no clamp or null
// text fallback. The Win32 CRT's actual invalid-parameter behavior is retained.
std::int32_t frame_job_processor_count_00be4800();

// ECX secondary pool, RET: only restore D68638. This is the complete state0
// unwind for both secondary construction and destruction; no arrays are freed.
void unwind_frame_job_pool_base_00be2c30(NativeFrameJobPoolStorage&) noexcept;
// ECX primary, RET: clear actual0109CF08 unconditionally, then publish CE3818.
void unwind_frame_job_owner_base_004b7a00(NativeFrameJobOwnerStorage&,
    NativeFrameJobOwnerStorage* volatile& actual_global_0109cf08) noexcept;

// BE2C70, ECX pool/stack processor count/RET4. Counts wrap as native DWORDs;
// worker_count = processors-1. Zero flags/jobs, allocate four arrays only for
// positive worker count (unsigned multiply overflow requests FFFFFFFF bytes),
// create two manual-reset events per worker, signal done, create SUSPENDED
// Win32 worker with actual pool argument, and request priority2. API failures
// are retained/ignored as native; no allocation/thread rollback is introduced.
void create_native_frame_job_workers_00be2c70(NativeFrameJobPoolStorage&,
    std::int32_t processor_count, NativeFrameJobLifetimeBindings&);
// BE3040, ECX pool/stack requested processors/RET4/EAX pool. Set D68650,
// clear +04/+08/+10 ONLY, zero jobs, resolve -1 via BE4800, then BE2C70.
// +0C preimage survives if no positive workers. Failure restores base identity
// only, leaving any allocated arrays/events/threads for the native caller.
NativeFrameJobPoolStorage* construct_native_frame_job_pool_00be3040(
    NativeFrameJobPoolStorage&, std::int32_t processors, NativeFrameJobLifetimeBindings&);

// BE2DA0, ECX pool/RET. Wait current done events, capture current virtual08,
// set stop1, dispatch(use_workers1), WaitForMultipleObjects(all, INFINITE),
// close handles, scalar-delete current events and clear captured cells AFTER
// calls. Free/clear CURRENT arrays in order start, done, handles, IDs. Count and
// flags are left stale. No local unwind, retry, count clamp or API-success fiction.
void stop_native_frame_job_workers_00be2da0(NativeFrameJobPoolStorage&,
    NativeFrameJobLifetimeBindings&);
// BE30C0 / BE31E0: publish D68650, stop, restore D68638 even on unwind;
// scalar flags&1 frees only after successful stop; EAX original, scalar RET4.
void destroy_native_frame_job_pool_00be30c0(NativeFrameJobPoolStorage&,
    NativeFrameJobLifetimeBindings&);
NativeFrameJobPoolStorage* delete_native_frame_job_pool_00be31e0(
    NativeFrameJobPoolStorage*, std::uint32_t flags, NativeFrameJobLifetimeBindings&);

// 4BFA40 ECX primary/RET/EAX primary: CE7520, construct secondary+04 with-1,
// then secondary CE7554 and primary CE7550. Only primary base cleanup on unwind.
NativeFrameJobOwnerStorage* construct_native_frame_job_owner_004bfa40(
    NativeFrameJobOwnerStorage&, NativeFrameJobOwnerStorage* volatile& actual_global_0109cf08,
    NativeFrameJobLifetimeBindings&);
// 4BFAD0 ECX primary/RET: destroy secondary+04 then clear global/base CE3818,
// including on unwind. Scalar4BFB30 flags1 frees primary after success; RET4.
void destroy_native_frame_job_owner_004bfad0(NativeFrameJobOwnerStorage&,
    NativeFrameJobOwnerStorage* volatile& actual_global_0109cf08, NativeFrameJobLifetimeBindings&);
NativeFrameJobOwnerStorage* delete_native_frame_job_owner_004bfb30(
    NativeFrameJobOwnerStorage*, std::uint32_t flags,
    NativeFrameJobOwnerStorage* volatile& actual_global_0109cf08, NativeFrameJobLifetimeBindings&);
// Complete8-byte secondary scalar adjustor: ECX-=4, tail-jump4BFB30. Return is
// the original PRIMARY allocation address, not the incoming secondary pointer.
NativeFrameJobOwnerStorage* delete_native_frame_job_secondary_004bfac0(
    NativeFrameJobPoolStorage*, std::uint32_t flags,
    NativeFrameJobOwnerStorage* volatile& actual_global_0109cf08, NativeFrameJobLifetimeBindings&);

// 4C1130 cdecl/RET: actual singleton domain lock/recheck, allocate138A8,
// construct/publish, get the SAME manager again/register current publication,
// unlock captured section/reload global. Constructor raw free precedes unlock;
// registration failure does not undo publication. Domain callbacks must dispatch
// this exact owner through current primary scalar4BFB30 with its actual bindings.
NativeFrameJobOwnerStorage* native_frame_job_singleton_004c1130(
    NativeFrameJobOwnerStorage* volatile& actual_global_0109cf08,
    SingletonLifetimeDomain& actual_domain_01090aa0, NativeFrameJobLifetimeBindings&);

// Actual storage and real Win32 operations, new C++ ABI. Original integer
// vtables/EH are not executable host ABI; application entry/job/scalar routes
// remain required. No game runtime validation is implied by this reconstruction.
} // namespace bsp
