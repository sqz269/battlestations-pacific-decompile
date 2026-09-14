#pragma once

#include <cstdint>

namespace bsp {
// C0C90E fastzero_I: actual two-argument cdecl SSE2 instruction engine.
// Intended caller domain: positive 128-byte multiple, 16-byte aligned writable
// nonwrapping extent and actual CPU/OS SSE2 support. No feature/size guard.
// Direct count>=128 performs floor(count/128) iterations; zero/sub128 still
// enters the first 128-byte store group and then underflows the loop count.
// Preserves EAX/EDX/EBX/ESI and EBP/EDI; ECX and XMM0 clobbered, DF unchanged.
void __cdecl zero_native_crt_aligned_128_chunks_00c0c90e(
    void* destination, std::uint32_t byte_count);

// C0C965 __VEC_memzero: actual three-slot cdecl dispatcher, middle slot unread.
// Returns original destination, preserves EBP/EBX/ESI/EDI, requires DF=0 and
// writable nonwrapping extents disjoint from active frames/argument slots.
// Actual SSE2 is required when bulk executes. A nonzero signed32 pointer
// remainder r writes prefix=16-r BEFORE recursion: 1..15 bytes for positive
// representations, 17..31 for negative ones. Ordinary bounded use requires
// prefix<=byte_count. Direct misaligned short/zero calls have no repair.
void* __cdecl zero_native_crt_vector_00c0c965(
    void* destination, int ignored_fill, std::uint32_t byte_count);

// Complete BF79F0 _memset behavior through a NEW four-argument cdecl interface.
// Borrows the actual stable canonical 0109EEA4 cell; supplies no state owner.
// Count zero bypasses fill and feature reads. Only low(fill)==0, count>=256
// reads the current word; a nonzero word selects the actual three-slot vector
// tail. EAX is zero AND live at that gate, and MOV restores it without changing
// comparison flags. The source caller removes 16 bytes after return.
// Returns destination; preserves EBP/EBX/ESI/EDI. Arithmetic flags and volatile
// registers are not an ordinary C++ result contract. DF is unchanged; DF=0 is
// required for STOS paths. Requires valid writable nonwrapping extents avoiding
// active source/provider frames and live argument/return slots, including the
// fourth slot, plus actual CPU/OS SSE2 support when the vector engine executes.
// No validation, overflow repair, exception/frame/bootstrap owner or catch.
// Faults and partial stores remain; new ABI/reference access and code/fault-PC
// locations are qualified. This is not a drop-in original three-argument ABI.
void* __cdecl fill_native_crt_bytes_00bf79f0(
    void* destination, int fill, std::uint32_t byte_count,
    const volatile std::uint32_t& actual_feature_word_0109eea4);
} // namespace bsp
