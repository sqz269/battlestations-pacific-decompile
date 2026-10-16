#pragma once

#include <cstdint>

namespace bsp {

// Whole 004D49B0 (54 bytes/25 instructions): first in ECX, last in EDX, two
// ignored stacked metadata words, RET8. Both captured endpoints remain fixed
// except the forward cursor, which advances by native 32-bit wrapping +10h.
void __fastcall destroy_native_dword_vector_header_range_004d49b0(
    void* first, const void* last,
    std::uint32_t unused_metadata0, std::uint32_t unused_metadata1);

// Whole 004D9E40 (22 bytes/7 instructions): two stacked first/last words, RET8.
// Native pushes ambient ECX and a duplicate last only as ignored child words.
// This new stdcall interface canonically defines those dead words as zero;
// it does not require a semantic owner or ambient register input.
void __stdcall destroy_native_dword_vector_header_range_dispatch_004d9e40(
    void* first, const void* last);

// Equal endpoints skip all header accesses. Each reached 16-byte header captures
// DWORD+4 once, conditionally frees that captured nonzero backing via the
// actual current-CRT service, then unconditionally clears +4,+8,+0Ch in order.
// No direct opaque+0 access, null-header skip, element traversal or rollback.
// Reached headers must remain valid/writable through free and all stores,
// backing must belong to the admitted current CRT, and the forward equality
// traversal must terminate through valid storage. Earlier visits remain
// observable to later visits; no range/header snapshot is introduced.
//
// These void Source interfaces do not model incidental native EAX: the empty
// core leaves incoming bits, the empty dispatcher retains last, otherwise the
// final backing/free residue remains. Current C++/CRT behavior does not establish
// original heap/CRT, private argument-frame aliases, FH3/SEH/fault behavior,
// registers/flags, mangling, drop-in binary ABI or game/runtime equivalence.
// Neither whole leaf has local EH/actions. Correct STL_inst/TagDispatch
// identities remain recorded; these descriptive names are provisional.

} // namespace bsp
