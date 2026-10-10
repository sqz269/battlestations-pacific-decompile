#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native allocation statistics constructors require MSVC Win32.
#endif

namespace bsp {

// Borrow distinct actual publication cells. The bindings stay fixed throughout
// the call; their volatile values are read at the native observation points.
struct NativeAllocationStatsConstructorContext {
    void* volatile& actual_manager_01090aa0;
    void* volatile& actual_allocation_stats_0109cefc;
};

// Complete BE2750[145]: native ECX receiver, no stack arguments, EAX original
// receiver, RET. Arm base cleanup before D685E0; capture the first manager's
// +10h section, enter/increment, then arm guard cleanup and publish. Resolve
// the second manager before reloading CURRENT stats for BD0C30 registration.
// Normal release uses the first captured section and stays armed through Leave.
void* construct_native_allocation_stats_base_00be2750(
    void* actual_receiver, NativeAllocationStatsConstructorContext& context);

// Complete BE2900[32]: native ECX receiver, no stack arguments, EAX original
// receiver, RET. Only after base success write D685F4, DWORD 40000000 at +4
// and zero at +8. The 40000000 word is preserved as raw bits, not a float cast.
void* construct_native_allocation_stats_00be2900(
    void* actual_receiver, NativeAllocationStatsConstructorContext& context);

// The caller owns valid, four-byte-aligned writable 12-byte receiver storage
// with valid uint32_t accesses at +0/+4/+8, and retains its lifetime. Receiver,
// context and both publication cells are disjoint. Actual managers, their raw
// vector fields, and captured section storage (Win32 section plus DWORD +18h)
// remain valid through every provider call and required cleanup. No concurrent
// conflicting accesses or provider-induced storage invalidation are supported.
//
// Source C++ escape before guard arming invokes genuine 00412430; after arming,
// genuine 00411EE0 runs first, then 00412430. A second escaping C++ cleanup
// exception terminates. Publication and completed registration effects remain:
// no rollback, retry, unregister, global clear or receiver allocation/free.
// The genuine manager getter retains its existing lazy manager creation policy.
// Profile DWORDs retain original identity, not callable rebuilt virtual tables.
//
// These ordinary Source APIs add explicit context arguments; original register
// ABI, private EH spills/FH3, hardware-fault cleanup, OS registration and cookie
// behavior are unproved. No current startup wiring, semantic AllocationStatsState
// replacement, virtual dispatch, actual owner/delete policy or gameplay proof.
} // namespace bsp
