#pragma once

#include "bsp/observer_lifetime.hpp"

namespace bsp {

// Actual F878FC owner: profile DWORD+00, raw 1Ch tracked section pointer+04.
// CE7548 contains ONLY slot0=4C4890. CE754C begins the distinct profile stamped by 004BF9A0.
using NativeMissionEntityLockOwner = NativeObserverLockOwner;

// Complete 4BD150[69]: native ECX raw8, EAX same owner, RET. Stamp CE7548,
// create the canonical malloc-backed BD1860 section, then publish section+04.
// A C++ exception clears the supplied publication and stamps CE3818, without
// touching the section preimage (C64D20 -> 4B7ED0).
NativeMissionEntityLockOwner* construct_native_mission_entity_lock_004bd150(
    void* actual_owner, NativeMissionEntityLockOwner* volatile& publication_00f878fc);

// Complete 4B7ED0[17]: ECX owner, RET. Clear publication before base stamp.
// This base unwind does not release section+04 or unregister the owner.
void unwind_native_mission_entity_lock_004b7ed0(NativeMissionEntityLockOwner&,
    NativeMissionEntityLockOwner* volatile& publication_00f878fc) noexcept;

// Complete source behavior of 4C1570[189]: native no inputs, EAX owner, RET.
// Borrow the application's actual raw01090AA0 publication, using the existing
// SoundLifetimeAccess raw mode. Capture first manager section, enter/increment,
// double-check, allocate8/construct, publish, second manager lookup/register,
// release captured section, reload publication. Fast path returns its first read.
// Constructor failure frees captured raw8 after base unwind. Registration failure
// retains the owner/publication and releases the captured manager section.
NativeMissionEntityLockOwner* get_native_mission_entity_lock_004c1570(
    void* volatile& actual_manager_publication_01090aa0,
    NativeMissionEntityLockOwner* volatile& publication_00f878fc);

// Complete 4C4860[35] and full PE span4C4890[55]. Native nondeleting ECX=owner,
// RET; scalar ECX=owner, flags DWORD on stack, EAX=captured owner, RET4.
// Stamp CE7548, canonical41CC80 release of actual slot+04, clear publication,
// stamp CE3818. Scalar frees only for flags&1 and returns the captured address.
// Destruction neither checks publication identity nor unregisters the owner.
void destroy_native_mission_entity_lock_004c4860(NativeMissionEntityLockOwner&,
    NativeMissionEntityLockOwner* volatile& publication_00f878fc) noexcept;
NativeMissionEntityLockOwner* delete_native_mission_entity_lock_004c4890(
    NativeMissionEntityLockOwner*, std::uint32_t flags,
    NativeMissionEntityLockOwner* volatile& publication_00f878fc) noexcept;

// One process-static F878FC cell, with explicit lifecycle. Wrapper receives the
// application's SAME actual manager cell. Shutdown dispatch must call the above
// scalar function with this publication. There is no private manager or drain.
NativeMissionEntityLockOwner* volatile& process_native_mission_entity_lock_00f878fc() noexcept;
NativeMissionEntityLockOwner* get_process_native_mission_entity_lock_004c1570(
    void* volatile& actual_manager_publication_01090aa0);

// New C++ source ABI, not a binary replacement or callable native vtable.
// C++ cleanup reproduces owner/section effects, not original FH3/SEH dispatch,
// hardware-fault handling or mutable stack-spill aliasing. Saved Ghidra body
// membership still excludes004C48BE[3] and 00C64F91[2]; PE bytes establish those
// continuations. See docs/NATIVE_MISSION_ENTITY_LOCK.md for evidence limits.
} // namespace bsp
