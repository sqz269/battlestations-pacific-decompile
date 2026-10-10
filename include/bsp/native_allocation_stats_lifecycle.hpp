#pragma once

#include "bsp/native_allocation_stats_constructor.hpp"
#include <cstdint>

namespace bsp {

// Complete BE27F0[153]: native ECX receiver, no stack arguments, RET; no
// semantic EAX result. Stamp D685E0 before arming base cleanup. Capture the
// first manager's +10h section, enter/increment, then arm guard cleanup.
// Resolve the second manager BEFORE reloading CURRENT stats for BCFCA0.
// Clear the actual publication only after unregister returns; release the
// first captured section while armed, then reset the receiver to CE3818.
void destroy_native_allocation_stats_base_00be27f0(
    void* actual_receiver, NativeAllocationStatsConstructorContext& context);

// Complete physical BE2890[30] and BE2930[30]: native ECX receiver, one
// stack DWORD of flags, EAX original receiver, RET4. Both call BE27F0
// directly, then free the original receiver only if flags bit zero is set,
// and return its address. Higher flag bits are ignored. BE28E0 is excluded.
void* delete_native_allocation_stats_base_00be2890(
    void* actual_receiver, std::uint32_t flags,
    NativeAllocationStatsConstructorContext& context);
void* delete_native_allocation_stats_00be2930(
    void* actual_receiver, std::uint32_t flags,
    NativeAllocationStatsConstructorContext& context);

// Reuse the constructor's borrowed actual cells and raw-storage lifetime,
// alignment, disjointness and manager/section validity contract. Destruction
// does not change receiver +4/+8. The current publication may differ from the
// receiver, and the two manager observations need not return the same pointer.
// A deleting call requires receiver storage owned by singleton_lifetime_free.
//
// Source C++ escape before guard arming runs genuine 00412430; after arming,
// genuine 00411EE0 receives the actual local eight-byte guard, then 00412430
// runs if guard cleanup returns. A second escaping C++ cleanup exception
// terminates. Completed unregister/clear effects remain. No retry, rollback,
// failure-path publication clear, or receiver free occurs. The existing
// manager getter retains its lazy manager allocation policy.
//
// These Source APIs add explicit context arguments. Native EH action/frame
// evidence establishes a conditional schedule, not these APIs' original
// register ABI, private spills, OS delivery, FH3, cookie or hardware-fault
// behavior. Profile DWORDs are identities, not rebuilt virtual tables.
// No process owner, deletion-map binding, startup wiring or game validation.
} // namespace bsp
