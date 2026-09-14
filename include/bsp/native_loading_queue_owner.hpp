#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native loading queue owner requires MSVC Win32.
#endif

namespace bsp {

struct NativeEventOwnerStorage;

// Actual 20h storage. Numeric table identity is not a callable source vtable.
// Jobs are actual 24h allocations; only their state DWORD at +0 is used here.
// No constructor/default member initializer runs before the recovered stores.
struct NativeLoadingQueueOwnerStorage {
    volatile std::uint32_t table_00;
    volatile std::uint32_t worker_stop_04;
    void* volatile worker_job_08;
    NativeEventOwnerStorage* volatile event_0c;
    void* volatile* volatile jobs_10;
    volatile std::int32_t count_14;
    volatile std::int32_t capacity_18;
    void* volatile worker_thread_1c;
};

inline constexpr std::uint32_t native_loading_queue_table_00ceb198 = 0x00ceb198;

// Complete normal body 4FDBA0[89]. Native ECX raw owner, EAX same owner, RET.
// Create the actual manual-reset unsignaled event before initializing +10..1C.
// Source C++ failure at event construction follows C68800 -> 4F93A0: clear
// CURRENT E18D4C unconditionally, then stamp this+0 with base CE3818. The
// caller owns outer-allocation cleanup; no event or queue destructor is added.
NativeLoadingQueueOwnerStorage* construct_native_loading_queue_owner_004fdba0(
    NativeLoadingQueueOwnerStorage* actual_owner,
    NativeLoadingQueueOwnerStorage* volatile& actual_publication_00e18d4c);

// Complete normal body 4FDE20[189]. Native no inputs, EAX owner, RET; this new
// source interface borrows the two actual mutable publication cells. Use the
// actual native manager services, never a SingletonLifetimeDomain projection.
// Capture first manager+10 once, enter/increment, recheck E18D4C, allocate20h,
// construct, publish BEFORE reacquiring manager and registering CURRENT owner.
// Slow return reloads publication AFTER decrement/leaving the captured section.
// Constructor failure frees captured raw allocation, then releases the guard.
// Registration failure retains publication/allocation and releases the guard.
NativeLoadingQueueOwnerStorage* get_native_loading_queue_owner_004fde20(
    void* volatile& actual_manager_publication_01090aa0,
    NativeLoadingQueueOwnerStorage* volatile& actual_publication_00e18d4c);

// Complete504850[19]. Native two ignored name arguments, RET8. Reacquire the
// current loader, read its current +10 array and first job, and write state2.
// No original-job capture, callback identity check, count guard or stop check.
// The added cell references make this a new C++ interface, not a RET8 thunk.
void native_loading_queue_on_filestore_ready_00504850(
    void* volatile& actual_manager_publication_01090aa0,
    NativeLoadingQueueOwnerStorage* volatile& actual_publication_00e18d4c,
    const void* ignored_first_name, const void* ignored_second_name);

// CompleteBD1920[50]. Native ECX event, raw AL result, RET. Read current +04
// HANDLE and wait0: signaled=>1; timeout/abandoned/failed=>0 (failed additionally
// calls GetLastError); every other status returns its unmodified low byte.
std::uint8_t poll_native_event_owner_00bd1920(const NativeEventOwnerStorage*) noexcept;

// Source-only owner entry coverage. Do not wire getter/callback into production
// or install a placeholder deleting dispatcher: registered CEB198 lifetime
// requires actual509FD0/50ACE0/5092E0 and the complete drain/resource domains.
// No owner RAII rollback is added. Native FH3/SEH, mutable EH-spill aliases,
// original CRT throw identities and hardware-fault cleanup are not reproduced.
} // namespace bsp
