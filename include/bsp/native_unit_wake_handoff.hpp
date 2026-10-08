#pragma once

namespace bsp {
// Whole raw 00815E20..00815ED5, 182 bytes: ECX=new unit, stack=old unit,
// RET4. Unused EDX preserves that native stack spelling. Both actual borrowed
// FAC-byte unit prefixes and every reached pose ancestor must remain valid.
// There is no null fallback: callers must supply a usable new and old owner.
//
// Capture old FA0/FA8 bits before either pose refresh; refresh new, then test
// old C8 freshly. Preserve all x87 float32 difference and sum spills before
// copying the inline BD0 ring. FA0/FA4/FA8 are that same ring's 3D0/3D4/3D8
// tail, never a second representation. Publish X, positive-zero Y, then Z.
// XMM0 ends zero, MXCSR unchanged. Native incidental EAX=new+F9C,
// EDX=old+FA4, ECX=0; no semantic return value is established.
//
// Any reached parent matrix multiply requires an EMPTY x87 stack (all eight
// slots free; TOP may be nonzero). Cached/root-only paths need one free slot.
// Production never resets floating state. Dirty ancestry must terminate;
// raw pose recursion rereads the real parent. There is no provider or view.
// Actual construction, ownership, synchronization, faults/unmasked exceptions,
// group/type/speed/detach integration and game behavior remain external.
void __fastcall handoff_native_unit_wake_00815e20(
    void* actual_new_unit, void* unused_edx, void* actual_old_unit) noexcept;
} // namespace bsp
