#pragma once

#include <cstdint>

namespace bsp {
struct NativeStringRawPoolContext;

// Complete00559E00 normal helper and00559E5A completed-prefix catch. Native
// ECX first, EDX last; stack(destination,dead_owner,dead_tag3,dead_tag4),
// RET10h, EAX completed destination end. Source retains those six physical
// input words and adds context (new Source RET14h/exceptionABI).
//
// Actual10h-byte checked-vector headers contain opaque/proxy+0, begin+4,
// end+8,capacity+C; inner strings are8 bytes. Construct each nonnull destination
// through complete actual00557BC0, then advance destination and source by10h
// with native DWORD wrap/equality. Null destination skips that constructor but
// STILL advances and joins the completed prefix. No source snapshot/self guard,
// range repair or +0 initialization is added. Destination is actual admitted
// construction storage, not a borrowed host vector or assignment target.
//
// On a C++ failure,00559E5A tidies completed headers in ascending order through
// the exact actual00559DC0/00432440 contract, then rethrows. Failing current
// header is excluded. Native state1 placement-delete action00401130 is RET-only;
// no allocating/freeing replacement action is invented. Cleanup failure stops
// later cleanup/rethrow and escapes in the qualified Source catch domain.
void* __fastcall copy_native_nested_string_vector_uninitialized_range_00559e00(
    const void* first, const void* last, void* destination,
    std::uint32_t dead_owner, std::uint32_t dead_tag3,
    std::uint32_t dead_tag4, NativeStringRawPoolContext& strings);

// Complete0055A950..0055A974. Native ECX dead_owner; FIVE physical stack words
// (dead_first_owner,first,dead_last_owner,last,destination), RET14h, EAX helper
// result. EDX input unconsumed. Source reserves that register, preserves stack
// positions, and adds context (new Source RET18h). Forwards helper words:
// first,last,destination,dead_owner,destination_bits,dead_owner&FFFFFF00h.
// Only low tag byte is zeroed in native; high24 retain incoming ECX. All three
// metadata words are proven dead in helper/catch and require no live owner.
void* __fastcall copy_native_nested_string_vector_dispatch_0055a950(
    std::uint32_t dead_owner_ecx, std::uint32_t unused_edx,
    std::uint32_t dead_first_owner, const void* first,
    std::uint32_t dead_last_owner, const void* last, void* destination,
    NativeStringRawPoolContext& strings);

// Native16-byte stepping must reach last and all executed child constructor,
// actual string/pool/current-CRT ownership and catch cleanup accesses must be
// admitted. Native null skip alone does not make a null completed-prefix slot
// safely tidyable after later failure: the catch has NO null skip. Overlap and
// child no-self-guard effects remain where those contracts hold. Outer backing
// is supplied by caller; this helper only constructs/tidies inner headers and
// never frees the outer allocation. Explicit context owns no storage/callback.
// Source by-value/private frames, added context and C++ catch/rethrow do not
// reproduce original FH3/SEH/CRT/catch identity or cleanup-failure unwind ABI.
// No native execution, binary replacement or gameplay claim.
} // namespace bsp
