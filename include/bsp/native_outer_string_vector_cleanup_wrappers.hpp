#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual outer string vector cleanup wrappers require MSVC Win32.
#endif

namespace bsp {
struct NativeStringRawPoolContext;

// Complete0055AC60..0055AC75. Native ECX receiver, EDX unconsumed;
// stack first/last, RET8, no semantic result. Source preserves those physical
// inputs and adds stacked context. Native loads last into EAX and EDX, pushes
// last then receiver as dead metadata, loads first into ECX, and calls actual
//0055A3A0. Source forwards the same values, with no receiver dereference.
void __fastcall destroy_native_outer_string_vector_range_dispatch_0055ac60(
    const void* receiver, std::uint32_t unused_edx, void* first, void* last,
    NativeStringRawPoolContext& strings);

// Complete0055B110..0055B14C. Native ECX actual16B header, incoming EDX
// unconsumed, no stack input, plain RET; no semantic result. Source passes
// added context in EDX. All21 native instructions equal accepted0055AFF0
// after address/rel32 normalization, including the same direct callees and
// null branch: captured range destruction, CURRENT begin free, unconditional
// ordered +4/+8/+C clears, no write to+0. Reuse that complete actual service.
void __fastcall tidy_native_outer_string_vector_0055b110(void* header,
    NativeStringRawPoolContext& strings);

// No new validation, callbacks, storage model, catch/retry or rollback.
// Accepted raw-pool/current-CRT child failures propagate and stop later work.
// A range with equal endpoints performs no element work. Source void APIs do
// not promise native register residue. Added context/by-value inputs do not
// reproduce native stack cleanup, private argument/spill aliases, register or
// FH3/SEH/CRT exception identity. Tail-jump unwind callers remain native-only
// evidence, not new Source frame adapters. Older qualified host/storage
// projections remain separate; native/gameplay validation is not claimed.
} // namespace bsp
