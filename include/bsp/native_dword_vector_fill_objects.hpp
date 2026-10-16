#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native DWORD vector object fill requires MSVC Win32.
#endif

namespace bsp {

// Complete STL_FillObjects_00698010, 00698010..00698035: 38B/18 starts.
// Native cdecl three stack words(first,last,source), plain RET. Capture first
// and last even when equal; capture the source-header pointer only after the
// nonempty check. Each iteration calls actual accepted 00697BD0 assignment
// with CURRENT source-header contents, advances destination address by 10h
// with DWORD wrapping, then repeats until equality with the captured last.
// No snapshot, typed-array subtraction/count conversion or extra guard.
void __cdecl fill_native_checked_dword_vector_objects_00698010(
    void* first, const void* last, const void* source);

// The void Source interface excludes native incidental EAX (unconsumed on
// empty input, otherwise left by the last child); producer sites do not use
// it. No original C++ return type is claimed. The child's incoming EDX is
// unconsumed, so Source passes zero without an ambient-register promise.
// No local EH, allocation, cleanup or rollback is added. Exceptions propagate
// through accepted Current C++/CRT services. Reached raw headers must remain
// accessible and the equality traversal finite. Original private argument/
// frame aliases, registers/flags, CRT/fault/SEH/FH3/RTTI/binary ABI and gameplay
// remain unproved. Prior qualified Source and fixtures retain their provenance.
} // namespace bsp
