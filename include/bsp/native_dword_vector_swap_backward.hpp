#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual DWORD-vector header backward swap requires MSVC Win32.
#endif

namespace bsp {

// Whole006981C0,67B/26instructions. Native cdecl consumes first,last,output_end;
// its actual producer pushes SIX physical words and removes18h. Slots4..6 are
// never read. PlainRET; EAX is the advanced backward output, including empty.
// Capture last and output before first/equality. For each iteration, load the
// two begin words at last-0Ch/output-0Ch BEFORE decrementing both cursors10h.
// Store destination begin then source begin; then separately load/store each
// +8 and+C pair in that same source/destination order. Never copy opaque+0.
// Public aliases observe this staged swap of three DWORD ownership words.
void* __cdecl swap_native_dword_vector_headers_backward_006981c0(
    void* first, void* last, void* output_end,
    std::uint32_t unused_output_bits,
    std::uint32_t unused_output_bits_again, std::uint32_t unused_tag);

// Whole00698380,64B/29instructions. Native cdecl has THREE stacked inputs and
// plainRET. Call the actual core with captured inputs, two duplicate output
// bit patterns and an unconsumed private tag. Ignore core EAX; independently
// return output-((SAR4(wrapped last-first))<<4), with DWORD wrapping arithmetic.
// Original incoming ECX supplies only the high24 dead tag bits after clearing
// its low byte. Source uses canonical private zero instead; no owner/context
// or native ambient-register/private-frame identity is asserted.
void* __cdecl swap_native_dword_vector_headers_backward_dispatch_00698380(
    void* first, void* last, void* output_end);

// Whole native normal bodies, no localEH or allocation/destruction/assignment
// child. Finite raw equality termination and valid reached accesses are input
// conditions, not new guards. No count bridge, typed vector/header snapshot,
// validation, callback, rollback or noexcept promise. Raw x86 DWORD accesses
// retain unaligned/public-alias order; Source private storage/argument slots,
// flags/register liveness, original faults/SEH/FH3/CRT, binaryABI and gameplay
// equivalence remain unproved. Existing library identities/qualified APIs stay
// separate; these descriptive Source names are hypotheses, not recovered names.

} // namespace bsp
