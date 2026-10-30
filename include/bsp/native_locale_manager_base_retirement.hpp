#pragma once

#include "bsp/native_locale_manager_constructor.hpp"

namespace bsp {

// Complete 7365E0[153] control body: Original ECX receiver, no stack arguments,
// plain RET and no meaningful EAX result. This ordinary void Source API borrows
// the SAME immutable context/reference identities used for native construction.
void destroy_native_locale_manager_base_007365e0(
    void* actual_receiver, const NativeLocaleManagerConstructorContext&);

// Caller supplies actual aligned/addressable receiver bytes, actual manager
// and F8BC4C cells, real raw manager/vector backing and captured physical Win32
// section+18 storage. Keep receiver, cells, context, sections and every affected
// registration/publication graph alive through every normal/escaping path.
// No concurrent conflicting writers, retargeting or invalidating callbacks.
// Raw DWORD accesses use fenced byte copies, not assumed C++ field objects.
// Genuine raw 411EE0/412430 cleanup (f411f273) is a required coupled dependency.
//
// Store CFEA5C BEFORE state0. Capture the FIRST manager's section; arm state1
// AFTER optional Enter/depth, including the null skip. Resolve the SECOND
// manager BEFORE loading CURRENT F8BC4C for the sole BCFCA0 call. Clear F8BC4C
// unconditionally ONLY after that call returns. Release the captured section
// while state1 remains armed, then genuinely reset the ORIGINAL receiver.
// Escaping normal Leave can therefore decrement depth again during cleanup.
// No rollback, unregister retry, equality guard, receiver free or new owner.
//
// Only selected state0/root and state1/guard cleanup are composed. A second
// escaping C++ cleanup terminates. Ordinary Source exceptions do not reproduce
// Native register ABI/FH3/SEH/hardware-fault/private-spill/throw identity rules.
// Canonical cell/profile/lifetime binding, full 73C9F0/scalar deletion, loading,
// startup, ABI/runtime compatibility and source-ready activation remain absent.

} // namespace bsp
