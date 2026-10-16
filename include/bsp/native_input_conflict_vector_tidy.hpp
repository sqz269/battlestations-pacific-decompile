#pragma once

#include <cstdint>

namespace bsp {
// Complete native 0069EEA0..0069EED5 and 006A1420..006A145C.
// The STL/container descriptions are hypotheses. These entries operate on
// actual 16B headers; each +4 backing pointer owns trivially destroyed data.
// Header +0 is opaque and retained. Raw DWORD stepping and accesses below
// require the reached native storage; there is no null-header/range guard.
//
// Core Native ABI: ECX first, EDX last, two ignored stacked DWORDs, RET8.
// Capture last once. For every reached header capture +4, conditionally free
// that captured pointer, then unconditionally clear +4/+8/+C in that order.
// Empty equality skips every element read; advance is unsigned DWORD +10h.
// Source void qualifies Native EAX residue; no semantic return is promised.
void __fastcall destroy_native_input_conflict_vector_header_range_0069eea0(
    void* first, void* last, std::uint32_t dead_header_word,
    std::uint32_t dead_private_word);

// Tidy Native ABI: ECX header, incoming EDX unused, no stacked words, RET.
// Capture first +4; only nonzero first reads +8/calls range. Afterwards reload
// CURRENT +4 for free, even if now zero; always clear +4/+8/+C, preserving +0.
// Native PUSH ECX saves incoming ECX/header bits in a compiler-private slot;
// the later MOV [ESP+4] reads those known bits. Both native child metadata
// words contain header bits, but the core ignores them. Source normalizes
// the second dead word to 0; this is not an original private-frame ABI claim.
void __fastcall tidy_native_input_conflict_vector_006a1420(
    void* header, std::uint32_t unused_edx);

// Current MSVC Win32 Source uses singleton_lifetime_free/current CRT. No local
// C++ catch, rollback or new cleanup guard is introduced. Raw access faults,
// original CRT/private frames/register residue, original-runtime binary ABI,
// FH3/SEH and game behavior remain unverified. Existing AV/typed projections
// and their valid storage/fixture domains remain separate and unchanged.
} // namespace bsp
