#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native DWORD vector assignment requires MSVC Win32.
#endif

namespace bsp {

// Complete STL_Vector_Assign_00697bd0, 00697BD0..00697CF4: 293B/133 starts.
// Original ECX destination, incoming EDX unconsumed, one stacked source-header
// pointer, RET4; EAX is the original destination on every normal return.
// Both actual 10h headers have opaque +0, begin+4, end+8, capacity_end+C.
// Exact self assignment returns before any header read. Null source begin
// skips source end; zero SAR2 size calls actual accepted 697890 clear.
//
// Shortening calls actual progressive 697080, then reloads source begin/end
// and destination begin to recompute the published end. Fits-capacity copies
// a captured prefix, reloads destination end then source end, copies the tail
// through actual 6974B0 and publishes its returned EAX. Growth frees captured
// old begin BEFORE actual 557200 Buy, recomputes count after free, tests only
// Buy AL, and reloads destination begin/source end/source begin after Buy.
// No rollback, extra guard, source snapshot, fill or element destruction.
// Unsigned comparisons, DWORD wrapping and SAR2 arithmetic are explicit.
void* __fastcall assign_native_checked_dword_vector_00697bd0(
    void* destination, std::uint32_t unused_edx, const void* source);

// Actual accepted raw services and current malloc/free/exception domain are
// used directly. No callbacks, header/iterator/allocator model or count bridge.
// This is a new Source interface, without a noexcept barrier. Private argument
// and frame aliases, dead register/flag residue, native CRT/SEH/FH3/RTTI/fault
// identity, installed binary ABI and gameplay remain unproved. Earlier actual
// storage fragments and their narrower fixtures keep their own provenance.
} // namespace bsp
