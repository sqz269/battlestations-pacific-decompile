#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual string vector insertion requires MSVC Win32.
#endif

namespace bsp {
struct NativeStringRawPoolContext;

// Complete0044F410 normal body and both metadata-selected catches0044F5C8/
//0044F653.0044F5E5 is the growth-catch rethrow, not an independent function;
//0044F6B5 is normal temporary cleanup inside0044F410, not a callee.
// Native fastcall: ECX actual10h vector, EDX unused; stacked unused_owner,
// position,count,value-header; RET10h; no semantic result. Source preserves
// these physical inputs and adds the actual raw-pool context. No owner,
// iterator, bounds, alignment, capacity-consistency or null-input guard is added.
//
// Copy value into a real8h temporary BEFORE reading vector begin/capacity or
// checking count, even when count0. Reuse the accepted actual0044BCB0 header
// construction schedule (zero header, compare identity, actual0041DD40 resize,
// CURRENT fields copy); its only extra null-destination guard cannot be taken
// by the private temporary. Temporary construction failure has no owner cleanup.
// After the first vector-begin read, all later C++ failures clean that temporary
// through actual0041DD20. The normal path disables that cleanup before release.
//
// Preserve captured begin/capacity, repeated CURRENT end reads, unsigned
// max-size subtraction and wrapping SAR3 sizes. Growth uses the exact guarded
// capacity+capacity/2 candidate and direct accepted0042BF40 allocation. Copy
// prefix, fill inserted elements, copy suffix; advance parent progress only
// after successful prefix/fill. Its catch destroys only that completed parent
// prefix, frees the new backing, then rethrows. After suffix success this catch
// is OFF: old destruction/current-begin free and capacity/end/begin publication
// can fail without freeing the completed new backing. No rollback is added.
//
// With spare capacity, both native general-insertion branches are complete.
// Short tail: uninitialized-copy tail to position+count*8 before arming the
// fill catch; fill CURRENT end, then add count*8 to CURRENT end and reload it.
// Fill failure destroys position+count*8..CURRENT end+count*8 and rethrows.
// Assignment follows after that catch is off. Long/equal tail: copy last count
// headers to old end, publish returned end, backward-assign the middle, fill
// the gap. Exceptions there only clean the temporary; no assignment rollback.
void __fastcall insert_native_string_header_vector_count_0044f410(void* header,
    std::uint32_t unused_edx, const void* unused_owner, void* position,
    std::uint32_t count, const void* value, NativeStringRawPoolContext& strings);

// Exact actual pooled-string/header services are composed directly. The old
// append-only storage fragment and its native fixtures remain separate. This
// new Source interface has extra context, by-value inputs and C++ /EHsc scopes;
// it does not reproduce original private-stack aliases, FH3/SEH/CRT exception
// identity, fault addresses, native ABI or cleanup-failure unwind personality.
// A cleanup exception stops subsequent work and escapes in the Source domain.
// Accepted current-CRT overlap-copy/zero-byte omission and real pool/manager
// requirements are inherited. No new-function differential or gameplay claim.

} // namespace bsp
