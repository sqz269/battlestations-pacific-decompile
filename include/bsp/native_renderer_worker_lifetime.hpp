#pragma once

#include "bsp/native_string_pool_storage.hpp"
#include "bsp/random_threads.hpp"

namespace bsp {

// Fixed host ABI addition. Borrow the application's actual pool adapter, whose
// publication, small-return gate and lifetime binding must outlive this owner.
// This context owns nothing and contains no replacement OS/allocator callbacks.
struct NativeRendererWorkerLifetimeContext {
    ActualNativeStringPoolStorage* strings;
};

// Full 00415270: ECX actual eight-byte string header, EAX same pointer, RET.
// Ordered length=0, data=0 stores; no release of any previous data.
void* __fastcall initialize_native_string_header_00415270(void* actual_header);

// Full 00BD1860, native cdecl/no arguments/EAX lock. Exact raw 1Ch allocation
// through the existing BF681B host service, real InitializeCriticalSection,
// then depth=0. A failed OS initialization gains no allocation cleanup.
TrackedCriticalSection* create_native_tracked_critical_section_00bd1860();

// Full 00B5E270 / 00B5E2F0, native ECX owner and RET; constructor returns owner.
// The new EDX argument supplies the fixed string-pool context. Owner storage
// must cover +00..+53; this is an accessed prefix, not an allocation-size claim.
// Five raw headers occupy +14..+3B; +46/+47 stay untouched. Construction is for
// raw storage: previous headers/locks are not released. Its only outer rollback
// destroys the five strings, leaking the first lock if the second creation
// throws. Destruction leaves the published lock/handle/header words unchanged.
void* __fastcall construct_native_renderer_worker_00b5e270(void* actual_owner,
    NativeRendererWorkerLifetimeContext* context);
void __fastcall destroy_native_renderer_worker_00b5e2f0(void* actual_owner,
    NativeRendererWorkerLifetimeContext* context);

// Full 00B5E050: ECX owner, RET. Nonzero current +50 enables the byte44/45 stop
// protocol under current +4C, Sleep(10) polling, then CloseHandle(current +50).
// No WaitForSingleObject, handle clear, worker creation, or ownership inference.
void __fastcall stop_native_renderer_worker_00b5e050(void* actual_owner);

// Fixed five-element C++ cleanup specializes the observed compiler loops.
// No generic BF7CD1/BF7C6E implementation or original CRT/SEH ABI is claimed.
// See docs/NATIVE_RENDERER_WORKER_LIFETIME.md for valid storage and exception
// boundaries, including the existing noexcept string-release adapter contract.
}
