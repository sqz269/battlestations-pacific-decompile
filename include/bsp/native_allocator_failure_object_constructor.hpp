#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native allocator failure-object construction requires MSVC Win32.
#endif

namespace bsp {

// Complete effects of 00BF6802..00BF681A (25 bytes), composed with the admitted
// 00BF638E raw 12-byte leaf. Descriptive Source name is provisional; the saved
// Native library name remains bad_alloc. This is an ordinary C++ Source API,
// not the Native ECX-receiver / no-stack-argument / plain-RET entry interface.
//
// Caller supplies actual live backing for 12 receiver bytes and an actual
// readable four-byte pointer-slot address. The borrowed explicit slot input
// substitutes the Native fixed 00E154B4 address; no cell producer, string,
// static owner or Native fixed-address binding is provided by this interface.
// Call the real base-message leaf with unused EDX word 0 and ignored numeric
// stack word 1. It publishes 00D69370h, reads the slot, performs the real DWORD
// read-modify-write at receiver+8 and stores the captured slot word at+4.
// After its normal return, unconditionally write raw 00D6923Ch at the captured
// receiver+0 and return that original receiver, ignoring the child's result.
// No null checks, pointee access, allocation, catch, cleanup or ownership
// transfer are added; the child retains alias/fault order and may leave partial
// effects. Raw profile words do not create a callable Source vtable, RTTI or
// std::bad_alloc identity. Actual backing/slot/pointee lifetime and wider
// exception/runtime identity remain caller contracts.
void* construct_native_allocator_failure_object_00bf6802(
    void* actual_raw12_receiver,
    const void* actual_pointer_slot_address);

} // namespace bsp
