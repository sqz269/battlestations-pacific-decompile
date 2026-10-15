#pragma once
#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual outer-vector erase range requires MSVC Win32.
#endif

namespace bsp {
struct NativeStringRawPoolContext;

// Complete006A5BF0. Native ECX receiver, five stacked words: output iterator,
// first owner, first position, last owner, last position; RET14h. EAX is the
// output iterator address after its two ordered stores. Source reserves EDX
// and adds actual pool context. Actual16B owning inner headers are assigned
// forward, then the captured resulting suffix is destroyed before publishing
// current receiver end. Opaque+0 and receiver begin/capacity are not changed
// directly. Output aliases retain the native final owner/position write order.
void* __fastcall erase_native_outer_vector_range_006a5bf0(void* header,
    std::uint32_t unused_edx, void* output_iterator, const void* first_owner,
    void* first_position, const void* last_owner, const void* last_position,
    NativeStringRawPoolContext& strings);

// Complete006A6310. Native ECX receiver, EDX unused, no stacked input, RET.
// Capture original end before first unsigned bounds comparison; capture begin
// after any returning first diagnostic, compare with CURRENT end, and retain
// both captures across the second diagnostic. Invoke the full erase range
// with a private two-word output iterator. Sourcevoid excludes private EAX.
void __fastcall clear_native_outer_vector_via_erase_006a6310(void* header,
    NativeStringRawPoolContext& strings);

// Diagnostics call accepted actual BF6713/current SDK policy and may return.
// No extra range/owner/null guard, catch, snapshot or rollback. All executed
// actual header/assignment/cleanup accesses and child ownership/pool/CRT
// contracts must be admitted. Private frame aliases are excluded. Added
// context/current CRT/C++ interfaces do not prove original register ABI,
// FH3/SEH/RTTI, diagnostic provider, faults or live gameplay equivalence.
} // namespace bsp
