#pragma once

namespace bsp {
struct TrackedCriticalSection;

// Full 0041CC80..0041CCBF: ECX addresses the actual four-byte owner slot;
// no stack arguments, plain RET, and no declared return value. Both the slot
// address and its section are captured once. A null section leaves the slot
// untouched. A nonnull section is a malloc-backed raw 1Ch TrackedCriticalSection
// paired with create_native_tracked_critical_section_00bd1860, not a projected
// section or an allocation made by the older new/delete convenience interface.
// Call on the owning/quiescent thread. No original CRT or hardware-fault/SEH
// compatibility is implied by this source ABI.
void __fastcall release_native_tracked_critical_section_0041cc80(
    TrackedCriticalSection** actual_owner_slot);
} // namespace bsp
