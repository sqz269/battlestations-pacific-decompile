#pragma once

#include "bsp/native_singleton_vector_allocation.hpp"

#include <cstdint>

namespace bsp {
// Complete 00432090..004320D6 at the existing Source CRT/exception boundary.
// Original ECX actual checked vector, stacked unsigned count, RET4; EDX unused.
// Reserve EDX explicitly in this fastcall signature. This is the 16-byte raw
// header: opaque/proxy+0, begin+4, end+8, capacity_end+Ch. It is NOT the other
// NativeStringVectorStorage 0Ch data/count/capacity layout. No overlay or C++
// vector constructor is introduced. Store zero to +4,+8,+C BEFORE either throw
// or allocation; +0 is not written. No old allocation is freed or rolled back.
// Count0 returns full EAX=0 without allocating. Count>1FFFFFFF throws the owning
// legacy length transport below. Otherwise allocate count*8 raw bytes, publish
// begin, then end=begin, then capacity_end=(native32 begin+count*8).
// Full EAX is exposed, not bool: (capacity_end & FFFFFF00h)|1. Low AL is success.
// Actual header storage must remain writable/live across real new-handler/EH
// activity; private frame/spill aliases and original FH3 ABI are not promised.
std::uint32_t __fastcall buy_native_checked_string_vector_00432090(
    void* actual_vector, void* unused_edx, std::uint32_t count);

// Complete 0042BF40..0042BF94 at the accepted current-CRT boundary. ECX count,
// no consumed EDX/stack input, EAX raw storage, plain RET. Unsigned max count
// is1FFFFFFF; above it uses the same Source std::bad_alloc boundary as accepted
// 00BCFEB0. Zero STILL calls allocation with0 bytes. Other admitted requests
// use exactly count*8 native/host bytes through actual singleton_lifetime_allocate
// (malloc/_callnewh/retry/throw), object byte-domain, matching singleton free.
// This does not bind the game's static CRT heap/new-handler or original
// D6923C/E03CC0 bad_alloc exception identity; no fabricated services/metadata.
void* __fastcall allocate_native_string_vector_0042bf40(
    std::uint32_t count, void* unused_edx);

// Complete 00431F70..00431FD8 via fully matched accepted 00BD0590 service.
// No input or normal return. Actual SBO capacity15/length0/NUL, counted18-byte
// message, 00411700 payload construction, D69260 address word, and temporary
// cleanup order are retained. Throws NativeSingletonVectorLengthError owning
// the actual28h legacy payload; NOT generic std::length_error. Source catch
// type/RTTI and C++ EH replace original D83F98/BF6885/FH3, as already qualified
// by that service. No extra SBO initialization or dropped side effects.
[[noreturn]] void __cdecl throw_native_string_vector_length_error_00431f70();

// Existing library-role Ghidra names are retained. No installed binary/native
// exception-ABI replacement, original CRT identity or gameplay claim.
} // namespace bsp
