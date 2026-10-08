#pragma once

namespace bsp {
// Whole raw 00815680..00815847: ECX=destination ring, stack=source ring,
// RET4. The unused EDX spelling retains the native source stack position.
// Both pointers borrow actual storage; there is no ring overlay or owner.
//
// Copy the integer head at +3C8 first, then 240 ascending float32 FLD/FSTP
// pairs at +8..+3C7. Preserve native partial-overlap and self-copy behavior,
// including SNaN quieting, denormal/invalid status and ambient x87 state.
// There is no snapshot, overlap guard, integer copy or FP environment change.
//
// Addressed storage must stay valid; one free x87 stack slot is required.
// Destination +0..7 and +3CC..3DB are untouched, including the actual vptr,
// owned critical-section handle, flag and residual. No handle is transferred.
// ESI is preserved; native incidental exits are EAX=destination+3CC,
// EDX=source+3D4 and ECX=0. No semantic return value is established.
// Full ring/unit construction, synchronization, unmasked faults/exceptions,
// outer wake/pose/group integration and game behavior remain external.
void __fastcall copy_native_unit_wake_00815680(
    void* actual_destination, void* unused_edx, const void* actual_source) noexcept;
} // namespace bsp
