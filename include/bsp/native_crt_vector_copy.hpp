#pragma once

#include <cstdint>

namespace bsp {
// Complete C0C7A4 native cdecl instruction engine. Requires actual SSE2
// support, 16-byte aligned readable/writable nonwrapping forward-safe extents,
// and a positive byte count divisible by 128. No short-size or feature guard.
// Preserves incoming EAX/EDX/EBX and EBP/ESI/EDI; clobbers ECX and XMM0..XMM7.
// DF is unchanged. The void declaration does not invent a return value.
void __cdecl copy_native_crt_aligned_128_chunks_00c0c7a4(
    void* actual_destination, const void* actual_source, std::uint32_t byte_count);

// Complete C0C82B native cdecl dispatcher. Returns the original destination.
// Requires DF=0, actual SSE2 support for the vector path, readable/writable
// nonwrapping forward-safe extents, and sufficient bytes for an alignment
// prefix when the two signed 32-bit pointer remainders modulo16 are equal
// and nonzero. That prefix is 16-remainder: 1..15 for positive representations,
// 17..31 for negative ones. The prefix must not exceed byte_count.
// Original BF87E0 callers additionally gate size>=256, current feature word,
// and matching low-bit alignment; this entry creates none of that state.
// Preserves EBP/EBX/ESI/EDI. Real cdecl recursion and the actual SSE2 engine
// remain instruction calls; no stack realignment, validation, catch or owner
// is supplied. Native memory/instruction faults and partial writes propagate.
void* __cdecl copy_native_crt_vector_00c0c82b(
    void* actual_destination, const void* actual_source, std::uint32_t byte_count);
} // namespace bsp
