#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual outer string vector cleanup requires MSVC Win32.
#endif

namespace bsp {
struct NativeStringRawPoolContext;

// Complete0055A3A0..0055A3E7. Native ECX first, EDX last, two entirely
// unused stacked words, RET8; no semantic return. Source preserves these
// physical words and adds stacked actual raw-pool context. Capture endpoints,
// walk by16 using DWORD wrap/equality, and compose accepted00432440 for each
// actual16B inner owning header. Empty range performs no field/pool/free work.
// Native duplicates original first into00432050's second unused stack word;
// the accepted child ignores both metadata words, permitting exact storage
// schedule reuse despite the distinct inner cleanup entry/stack conventions.
void __fastcall destroy_native_outer_string_vector_range_0055a3a0(void* first,
    void* last, const void* unused_owner, std::uint32_t unused_tag,
    NativeStringRawPoolContext& strings);

// Complete0055AFF0..0055B02C. Native ECX outer actual16B header, incoming EDX
// unconsumed, no stacked input, plain RET; no semantic result. Source adds
// context in EDX. Capture begin, then end if nonnull; destroy the captured16B
// header range, free CURRENT outer begin, then always zero+4,+8,+C in order.
// +0 is not written. Null begin skips end read/range/free but still clears.
void __fastcall tidy_native_outer_string_vector_0055aff0(void* header,
    NativeStringRawPoolContext& strings);

// Actual inner cleanup uses the existing raw string publications/pool and
// current CRT free. Per element: captured begin/end string destruction,
// CURRENT begin free, ordered field clears. No local catch/retry or rollback;
// a child exception stops later elements, outer free and later clears. No new
// owner, alignment, range, capacity, null or alias validation is introduced.
// Old qualified host/storage projections remain separate. Added context and
// current Source C++ behavior do not prove native ABI/private-stack/register,
// original CRT/FH3/SEH/exception identity, runtime or gameplay compatibility.
} // namespace bsp
