#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual string vector group cleanup requires MSVC Win32.
#endif

namespace bsp {
struct NativeStringRawPoolContext;

// Complete006A6EE0..006A6F27. Native ECX first, EDX last; two entirely
// unconsumed stacked DWORDs, RET8; no semantic result. Source retains these
// physical words and adds stacked context. Capture endpoints; walk by16 with
// DWORD wrap/equality. Each actual16B outer header owns actual16B inner checked
// vectors containing8B strings. Its full per-element schedule equals accepted
//0055AFF0: capture range, destroy, free CURRENT begin, always clear4/8/C, no
// write0. Equal endpoints perform no header/pool/free work.
void __fastcall destroy_native_string_vector_group_range_006a6ee0(void* first,
    void* last, std::uint32_t unused_stack0, std::uint32_t unused_stack1,
    NativeStringRawPoolContext& strings);

// Complete006A7190..006A71A5. Native ECX receiver word, EDX unconsumed;
// stack first/last, RET8; no semantic result. Native loads last twice into
// EAX/EDX and forwards receiver/last as dead range metadata. Source retains
// the physical words plus context; no receiver dereference or owner policy.
void __fastcall destroy_native_string_vector_group_range_dispatch_006a7190(
    std::uint32_t receiver_word, std::uint32_t unused_edx, void* first,
    void* last, NativeStringRawPoolContext& strings);

// Native006A6EE0 duplicates original first into0055A3A0's second dead stack
// word;0055AFF0 duplicates current header.0055A3A0 consumes neither metadata
// word, so accepted complete Source reuse preserves the storage schedule.
// Current raw-pool/CRT child exceptions stop later elements/free/clears, with
// no local catch, retry, rollback or added validation. +0 is opaque/unwritten.
// These context/by-value Source interfaces do not promise native stack/register
// residues, private-frame aliases, original heap/CRT/FH3/SEH/exception identity
// or native/gameplay compatibility. Existing typed settings projection/fixtures
// and host projections remain separate and unchanged.
} // namespace bsp
