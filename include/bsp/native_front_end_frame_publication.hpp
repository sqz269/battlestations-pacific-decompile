#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw front-end Frame publication requires MSVC Win32.
#endif

namespace bsp {

// Pure Source custody for a caller-prepared, retained getter invocation.
// These callbacks are noexcept METADATA ONLY: no native call, allocation,
// publication/receiver writes, retain, initialization, cleanup or reentry.
// Every record/backing exists before entry and survives all manager callbacks.
// Record the captured identities, never infer them from current publication.
struct NativeFrontEndFramePublicationCustody {
    virtual ~NativeFrontEndFramePublicationCustody() = default;
    virtual void allocation_returned(void* captured_allocation) noexcept = 0;
    virtual void construction_returned(void* captured_allocation,
        void* constructed_return) noexcept = 0;
    virtual void lifetime_argument_captured(void* captured_current_plus8) noexcept = 0;
    // Called AFTER the existing captured-allocation free; identity only.
    virtual void allocation_freed(void* captured_allocation) noexcept = 0;
};

// Complete004C1AC0 [202B/56ops]. Native consumes no argument, returns EAX,
// plain RET. Source borrows the SAME genuine distinct manager/Frame publication
// cells as the caller's actual singleton and Frame lifetime owners. No new
// global, alternate manager, copied publication or authentication is supplied.
//
// Fast return is the first captured Frame value. Slow path captures the actual
// manager+10 section, enters/increments +18 before state0, rechecks Frame,
// allocates164 before state1 and frees only the captured allocation on covered
// constructor failure. Publish the returned pointer after state1; capture the
// CURRENT Frame's +8/null BEFORE the second manager lookup/real registration.
// Keep the original section through normal decrement/leave, then return the
// CURRENT Frame. A covered C++ failure invokes full real411EE0 on the actual
// eight-byte CE37FC/section guard, then rethrows; no publication rollback.
void* get_native_front_end_frame_004c1ac0(
    void* volatile& actual_manager_publication_01090aa0,
    void* volatile& actual_frame_publication_00e18d80);

// SAME body and genuine two publication references; adds only the nonthrowing
// custody observations above. No extra native operation, early getter, Page
// initialization, changed catch, rollback/reset/retry or successful fallback.
// Observe allocation return (including null), constructor return before publish,
// captured CURRENT+8/null before second manager lookup, and existing constructor-
// failure free after it returns. Fast/existing arm emits no new allocation epoch.
// A registration failure leaves its published/older identities with the caller's
// preprepared records even if callbacks replace the CURRENT publication.
void* get_native_front_end_frame_004c1ac0(
    void* volatile& actual_manager_publication_01090aa0,
    void* volatile& actual_frame_publication_00e18d80,
    NativeFrontEndFramePublicationCustody& preprepared_custody);

// Real current CRT allocation/free, raw constructors/singleton registration,
// Win32 section/physical +18 counter and 411EE0 providers are composed directly.
// Stable distinct publication cells, valid current objects/section storage and
// provider lifetimes are caller proof, including through callbacks/reentry.
// No Page-array/current-set/selector/screen-register initialization is added.
// New ordinary C++ ABI and catch scopes: Native FH3/SEH/private spill aliases,
// hardware faults, secondary unwind/provider identities and game admission
// remain unproved. Failure leaves native completed publication/child effects.

} // namespace bsp
