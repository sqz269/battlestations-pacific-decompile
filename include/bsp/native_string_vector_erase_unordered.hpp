#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual unordered string vector erase requires MSVC Win32.
#endif

namespace bsp {
struct NativeStringRawPoolContext;

// Complete 0043EA00..0043EA72, actual0Ch vector / actual8h string headers.
// Native ECX=vector, stack=address of one iterator-pointer cell, RET4; no
// semantic result. Source adds the concrete raw-pool context in EDX.
// Capture count, data, then *iterator_cell. If the iterator differs from the
// captured last header, resize it from that last string and copy its CURRENT
// content. Recompute the last header from CURRENT vector count/data, release
// its nonnull buffer without clearing the header, then decrement CURRENT
// count. Capacity/backing and the iterator cell are not written explicitly.
// No validation, iterator advancement, empty guard, rollback or catch is added.
// The two native equal-last tests and all external header reloads are retained.
// Uses accepted0041DD40/00419CC0/BD1510 actual raw-pool bodies. Source skips a
// zero-byte CRT copy but retains its header reads, and uses memmove because
// native BF7680 admits overlap. Getter exceptions propagate before decrement.
// Extra context, inherited pool/CRT domain and lack of native private-frame,
// binary ABI, fault-address and game proof are explicit Source qualifications.
void __fastcall erase_native_string_vector_unordered_0043ea00(
    void* actual_vector, NativeStringRawPoolContext& strings,
    const void* iterator_cell);

} // namespace bsp
