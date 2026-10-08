#pragma once

namespace bsp {

// Complete raw 0070D980..0070D9F4: 116 bytes, 36 instructions, no calls.
// ECX is the actual group; entry EDX is unused. Two stack words are the
// actual writable 12-byte output and the integer member identity. Returns
// that same output address in EAX with RET 8; preserves ESI and EDI.
// The name and the role of the three record floats remain hypotheses.
//
// Borrows aligned 0x508-byte native group storage. Captures signed count at
// +0x4F8 once; positive count requires all reached 0x34-byte records at +0x18
// to exist (bounded ordinary domain: 1..24). First matching DWORD wins,
// including zero; member identities are never dereferenced as class objects.
// Copies record+0x04/+0x08/+0x0C through the exact sequential FLD/FSTP float
// schedule. Output may alias reached source storage: preserve that order,
// ambient x87 CW and conversion/exception effects. A hit needs one free
// x87 slot; this function returns no value in ST0 and adds no guard or clamp.
//
// No match requires ACTUAL readable native DATA at 0x00F87574/78/7C. If
// output aliases those cells, the aliased destination must also be writable.
// These addresses lie in writable .data's PE loader-zero tail. Neither the
// zero loader image nor saved-program zeros establish runtime immutability,
// current values, provider ownership or native global/world admission.
// An explicitly qualified standalone fixture may map a PE LOADER-image
// snapshot at those exact VAs; that admits only the fixture's frozen data.
// Production creates no mapping, default, replacement global or provider.
// Original class/lifetime, concurrent mutation, unmasked faults, enclosing
// caller and gameplay remain unbound.
void* __fastcall copy_native_unit_group_member_vector_0070d980(
    void* actual_group, void* unused_edx, void* actual_output,
    const void* actual_member) noexcept;

} // namespace bsp
