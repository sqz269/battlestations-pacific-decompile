#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual DWORD-vector header InsertOne requires MSVC Win32.
#endif

namespace bsp {

// Whole 006988F0, 139 bytes/57 current starts; established library identity
// STL_Vector_InsertOne_006988f0. Native ECX receiver, EDX unused, four stacked
// words: raw output-pair pointer, iterator-owner bits, position, fill-header
// pointer. RET10h; EAX is the actual output pointer, not child698400 residue.
void* __fastcall insert_native_dword_vector_header_one_006988f0(
    void* receiver, std::uint32_t unused_edx, void* output,
    void* iterator_owner, void* position, const void* fill_header);

// Direct receiver reads are raw +4 begin/+8 end. If begin is zero or the
// wrapped SAR4(end-begin) is zero, index=0 and both initial diagnostics skip.
// Otherwise initial begin/end and owner bits stay captured across returning
// diagnostics; index uses that initial begin. Actual698400 inserts count1.
//
// Capture new begin after the child and retain it across the next diagnostic.
// Test result against CURRENT end first; an upper failure skips the CURRENT
// begin lower-bound read. A returning report keeps the captured result.
// Store output+4 position BEFORE output+0 receiver, then return output.
// Public aliases follow those ordered raw operations; opaque receiver+0 is
// not directly read, but output/child aliases can still affect public bytes.
//
// Actual BF6713 Source calls the current SDK invalid-parameter policy, with
// neither noexcept nor noreturn promised. No handler installation, extra
// guard, owner object, count/container bridge, callback, snapshot, catch,
// rollback or cleanup is added. All reached accesses must be valid; DWORD
// addresses/subtractions/shifts/additions wrap as native32-bit values.
//
// This new Source interface uses distinct private arguments/stack/registers.
// Canonical child unusedEDX0 does not identify native EDX (fill pointer bits).
// Original argument/private-frame aliases, flags, CRT/heap/handler policy,
// FH3/SEH/fault/RTTI identity, binary ABI and game behavior remain unproved.
// Existing narrower storage/Hook interfaces and library names remain separate.

} // namespace bsp
