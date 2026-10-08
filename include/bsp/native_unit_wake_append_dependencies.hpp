#pragma once

namespace bsp {

// Raw 0042B2F0: ECX points at three readable float cells; ST0 returns the
// float-spilled length, ECX returns its raw float bits and RET consumes no stack arguments.
// Preserve the actual Y/X/Z load order, x87 arithmetic, float spills and
// FCOMI cutoff: squared length <= double 1e-10 or unordered returns +0.
// The other arm calls the genuine current CRT _CIsqrt with its input in ST0.
// Four free x87 slots are needed by this body; CRT entry requirements and its
// dispatch/diagnostic/exception policy remain external. No FP reset is added.
// Storage lifetime, validity and synchronization belong to the caller. This
// is a raw Win32 entry, not a typed trail adapter or Original CRT replacement.
float __fastcall native_unit_wake_length_0042b2f0(
    const float* actual_vector) noexcept;

// Raw 00810160: ECX=destination, [ESP+4]=source, EAX=destination on return,
// RET4. The unused EDX parameter preserves the native source stack position;
// it is not an object/context argument. ECX becomes source; EDX is untouched.
// Six ascending FLD/FSTP pairs access 24 readable source / writable destination
// bytes. Overlap and self-copy retain native sequential propagation, SNaN
// quieting, rounding/status effects and ambient x87 exception behavior.
// One free x87 slot is required. No snapshot, byte copy, overlap guard or FP
// reset is introduced. Caller owns valid storage and synchronization; this
// leaf constructs no ring, owner, reset vector or entity and updates no head.
void* __fastcall copy_native_unit_wake_sample_00810160(
    void* actual_destination, void* unused_edx, const void* actual_source) noexcept;

} // namespace bsp
