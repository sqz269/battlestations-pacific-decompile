#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native DWORD vector buy/copy requires MSVC Win32.
#endif

namespace bsp {

// Complete 00557200..00557246. Original ECX is an actual 10h header; EDX is
// unused. One stacked unsigned count, RET4. Clear +4, +8, +C in that order
// before the count0/length/allocation branches; +0 is not written. Count0
// returns EAX0 without allocation. Count > 3FFFFFFF uses accepted 556C20;
// otherwise the unchanged count reaches accepted 553260 with EDX0. Publish
// allocated begin, then end=begin, then wrapped capacity_end=begin+count*4.
// Native AL is success; upper24 EAX bits retain capacity_end. This uint32
// result exposes (capacity_end & FFFFFF00)|1, not a recovered C++ return type.
// No old allocation is freed or rolled back. Length/allocation failure occurs
// after the three clears in the accepted Current C++/CRT domain; intervening
// allocator/new-handler side effects are not rolled back either.
std::uint32_t __fastcall buy_native_checked_dword_vector_00557200(
    void* header, std::uint32_t unused_edx, std::uint32_t count);

// Complete 00697080..006970AE. Original cdecl with three stack words
// (first,last,destination), plain RET; caller removes 0Ch. Capture all three
// inputs even when empty. Precompute EAX=destination+SAR2(wrapped last-first)*4
// before data accesses. If nonempty, capture displacement=destination-first;
// read each CURRENT source DWORD, store through displacement+current source,
// then advance source4 until captured last. Return the independent result.
// No null-output skip, guard, memmove/snapshot, typed-pointer subtraction or
// count conversion is added.
void* __cdecl copy_native_dword_range_forward_00697080(
    const void* first, const void* last, void* destination);

// Explicit raw DWORD MSVC Win32 Source. Ordered machine accesses and unsigned
// arithmetic preserve these phases. Original private argument/frame aliases,
// ambient registers, fault/SEH/RTTI/CRT identity, binary ABI and gameplay remain
// unproved. Existing typed/callback Source and fixtures stay separate. No new
// allocator/owner/header/iterator model or arbitrary service is introduced.
} // namespace bsp
