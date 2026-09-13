#pragma once

#include "bsp/observer_lifetime.hpp"

namespace bsp {

// Actual F899E8 owner: profile DWORD+00, raw 1Ch tracked section pointer+04.
// D190C4 contains ONLY slot0=9256D0. D190C8 belongs to the distinct F899EC owner.
using NativePendingEntityLockOwner = NativeObserverLockOwner;

// Complete 924180[69]: native ECX raw8, EAX same owner, RET. Stamp D190C4,
// create the canonical malloc-backed BD1860 section, then publish section+04.
// A C++ exception clears the supplied publication and stamps CE3818, without
// touching the section preimage (CA6AE0 -> 923660).
NativePendingEntityLockOwner* construct_native_pending_entity_lock_00924180(
    void* actual_owner, NativePendingEntityLockOwner* volatile& publication_00f899e8);

// Complete 923660[17]: ECX owner, RET. Clear publication before base stamp.
// This base unwind does not release section+04 or unregister the owner.
void unwind_native_pending_entity_lock_00923660(NativePendingEntityLockOwner&,
    NativePendingEntityLockOwner* volatile& publication_00f899e8) noexcept;

// Complete source behavior of 9248D0[189]: native no inputs, EAX owner, RET.
// Borrow the application's actual raw01090AA0 publication, using the existing
// SoundLifetimeAccess raw mode. Capture first manager section, enter/increment,
// double-check, allocate8/construct, publish, second manager lookup/register,
// release captured section, reload publication. Fast path returns its first read.
// Constructor failure frees captured raw8 after base unwind. Registration failure
// retains the owner/publication and releases the captured manager section.
NativePendingEntityLockOwner* get_native_pending_entity_lock_009248d0(
    void* volatile& actual_manager_publication_01090aa0,
    NativePendingEntityLockOwner* volatile& publication_00f899e8);

// Complete 9256A0[35] and full PE span9256D0[55]. Native nondeleting ECX=owner,
// RET; scalar ECX=owner, flags DWORD on stack, EAX=captured owner, RET4.
// Stamp D190C4, canonical41CC80 release of actual slot+04, clear publication,
// stamp CE3818. Scalar frees only for flags&1 and returns the captured address.
// Destruction neither checks publication identity nor unregisters the owner.
void destroy_native_pending_entity_lock_009256a0(NativePendingEntityLockOwner&,
    NativePendingEntityLockOwner* volatile& publication_00f899e8) noexcept;
NativePendingEntityLockOwner* delete_native_pending_entity_lock_009256d0(
    NativePendingEntityLockOwner*, std::uint32_t flags,
    NativePendingEntityLockOwner* volatile& publication_00f899e8) noexcept;

// One process-static F899E8 cell, with explicit lifecycle. Wrapper receives the
// application's SAME actual manager cell. Shutdown dispatch must call the above
// scalar function with this publication. There is no private manager or drain.
NativePendingEntityLockOwner* volatile& process_native_pending_entity_lock_00f899e8() noexcept;
NativePendingEntityLockOwner* get_process_native_pending_entity_lock_009248d0(
    void* volatile& actual_manager_publication_01090aa0);

// New C++ source ABI, not a binary replacement or callable native vtable.
// C++ cleanup reproduces owner/section effects, not original FH3/SEH dispatch,
// hardware-fault handling or mutable stack-spill aliasing. Saved Ghidra body
// membership still excludes9256FE[3] and CA6B71[2]; PE bytes establish those
// continuations. See docs/NATIVE_PENDING_ENTITY_LOCK.md for evidence limits.
} // namespace bsp
