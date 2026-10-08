#pragma once

namespace bsp {

// Whole native 004F1830..004F1870 (64 bytes / 15 instructions / no calls).
// Hypothetical name; this is a raw memory interface, not a recovered class.
// actual_ecx supplies stable writable caller-owned storage of at least 0x18
// bytes. The actual native addresses 00F87574/78/7C must be readable. This
// function neither maps nor substitutes their runtime values. If the output
// aliases native data, that output must also be writable; load/store order is
// literal, including overlapping writes. There are no checks or fallbacks.
// unused_edx is formal fastcall padding: the native body preserves entry EDX.
// Returns the identical receiver in EAX; clears ECX and all 128 bits of XMM0;
// RET 0. Other XMM registers, x87, MXCSR, nonvolatiles and DF are unchanged.
// Final XOR defines CF=0, PF=1, ZF=1, SF=0, OF=0; AF is undefined.
void* __fastcall initialize_native_command_target_004f1830(
    void* actual_ecx, void* unused_edx) noexcept;

}  // namespace bsp
