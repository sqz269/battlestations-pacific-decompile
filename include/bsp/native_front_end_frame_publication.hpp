#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw front-end Frame publication requires MSVC Win32.
#endif

namespace bsp {

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

// Real current CRT allocation/free, raw constructors/singleton registration,
// Win32 section/physical +18 counter and 411EE0 providers are composed directly.
// Stable distinct publication cells, valid current objects/section storage and
// provider lifetimes are caller proof, including through callbacks/reentry.
// No Page-array/current-set/selector/screen-register initialization is added.
// New ordinary C++ ABI and catch scopes: Native FH3/SEH/private spill aliases,
// hardware faults, secondary unwind/provider identities and game admission
// remain unproved. Failure leaves native completed publication/child effects.

} // namespace bsp
