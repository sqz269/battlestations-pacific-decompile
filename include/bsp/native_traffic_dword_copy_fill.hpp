#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native traffic DWORD copy/fill requires MSVC Win32.
#endif

namespace bsp {

// Whole exact-address raw entries. Each has three actual stacked DWORD cells;
// no incoming ECX/EDX receiver or opaque class input. The caller's ECX header
// preimage is overwritten, not consumed. All return the actual EAX word below.
// Readable literal instructions retain measured caller-cell capture, register,
// flag, wrap and fault order; no byte embedding or count-matching padding.
//
// 00486800..0048681D,30B: plain RET/caller cleanup; EAX=end. Capture first/end,
// read the value-cell ADDRESS only if unequal, then reload its DWORD contents
// every iteration, store, advance by wrapped4 until exact endpoint equality.
void* __cdecl fill_native_traffic_dwords_00486800(
    void* first, const void* last, const void* value_cell);

// 00486820..0048684E,47B: plain RET/caller cleanup; EAX=destination_end-span.
// span=wrapped4*SAR2(wrapped(last-first)); copy only when signed count>0.
void* __cdecl copy_native_traffic_dwords_backward_00486820(
    const void* first, const void* last, void* destination_end);

// 0048D660..0048D68F,48B: RET0C; EAX=destination+span. SAR flags survive LEAs
// and PUSH. Every NONZERO signed count invokes the real copy, including a
// negative count with its wrapped DWORD byte span; there is no positivity repair.
void* __stdcall copy_native_traffic_dwords_0048d660(
    const void* first, const void* last, void* destination);

// 0048E4A0..0048E4CD,46B: RET0C; EAX=first+wrapped4*count. DWORD count uses
// unsigned nonzero TEST/JBE/JA, including high-bit counts. Read the value-cell
// ADDRESS only for a nonzero count; reload its DWORD contents every iteration.
void* __stdcall fill_n_native_traffic_dwords_0048e4a0(
    void* first, std::uint32_t count, const void* value_cell);

// Copy entries call actual current SDK ::memmove_s(dst,span,src,span), ignoring
// its result. Selected SDK errno/handler/copy policy is a qualified Source
// boundary, not original private CRT/EH/errno identity. No extra validation,
// snapshots, mutable globals, provider callbacks, noexcept or noreturn promise.
// Reached cells require real backing, including aliases and actual stack cells.
// Complete Original ABI/drop-in replacement, parent closure and game proof are
// unclaimed. See docs/GOAL15R_TRAFFIC_DWORD_COPY_FILL.md.

} // namespace bsp
