#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual string vector group tidies require MSVC Win32.
#endif

namespace bsp {
struct NativeStringRawPoolContext;

// Complete006A6CE0..006A6D1E. Native ECX/EDX unconsumed, stacked actual16B
// outer header, RET4; no semantic result. Source retains those physical words
// and adds context after the stacked header. Full schedule equals accepted
//0055AFF0 after adapting the stack input/prologue/epilogue: captured16B range,
// CURRENT begin free, always clear4/8/C, no write0. No null-header guard.
void __fastcall tidy_native_outer_string_vector_stacked_006a6ce0(
    std::uint32_t unused_ecx, std::uint32_t unused_edx, void* header,
    NativeStringRawPoolContext& strings);

// Complete006A75F0..006A762C. Native ECX actual16B group-vector header,
// EDX unconsumed, no stack input, plain RET; no semantic result. Source adds
// context in EDX. Capture begin and, if nonnull, end; destroy actual outer
// headers through006A6EE0, free CURRENT begin, always clear+4/+8/+C in order.
// Null begin skips end/range/free but still clears. +0 is not written.
void __fastcall tidy_native_string_vector_group_vector_006a75f0(void* header,
    NativeStringRawPoolContext& strings);

// Complete006A7960..006A799C, same native interface. All21 instructions and
//53nonrel32 bytes equal006A75F0, with the same range/free targets. Reuse the
// complete actual implementation, retaining its exact load/store schedule.
void __fastcall tidy_native_string_vector_group_vector_006a7960(void* header,
    NativeStringRawPoolContext& strings);

// Accepted actual raw-pool/current-CRT services are composed directly. No new
// callbacks, guards, allocation model, catch/retry or rollback. Child failures
// stop later destruction/free/clears and preserve earlier effects. Older typed
// settings/host projections and fixtures remain separate. Added context,
// by-value physical words and Source C++ behavior do not reproduce original
// stack/private-frame/register, heap/CRT/FH3/SEH/fault or exception identity.
// No native/gameplay proof or independently callable Source unwind adapter.
} // namespace bsp
