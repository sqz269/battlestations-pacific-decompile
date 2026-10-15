#pragma once

#include "bsp/native_avoid_manager_lifetime.hpp"

namespace bsp {

// Stable borrowed addresses of the actual three publication cells. Values may
// change during calls; no private publication, domain or replacement owner.
struct NativeAvoidSingletonContext {
    void* volatile* actual_01090aa0_publication_cell;
    Cc13AvoidZoneManagerNative** actual_00e17624_publication_cell;
    void* const* actual_00e188a8_publication_cell;
};

// Complete 004218E0 normal fast/slow schedules and recovered allocation/guard
// unwind operations through real current providers. Native entry has NO args,
// EAX result and RET. This new Source cdecl interface adds the explicit context
// reference; original FH3/frame/caller ABI is not transplanted.
Cc13AvoidZoneManagerNative* __cdecl get_native_avoid_manager_004218e0(
    const NativeAvoidSingletonContext& context);

// Registration uses the real raw singleton vector. The constructed manager's
// real Source deleting table REQUIRES EDX lifetime context; original raw-LIFO
// CALL EDX does not provide it. The finite current Source singleton dispatcher
// can call the real scalar with an explicit avoid_manager lifetime binding.
// This getter installs no such binding or production shutdown wiring and does
// not alter the registered pointer/profile. Original raw dispatch is distinct.

} // namespace bsp
