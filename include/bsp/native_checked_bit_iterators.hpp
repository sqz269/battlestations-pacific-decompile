#pragma once
#include <cstdint>
#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual checked bit iterators require MSVC Win32.
#endif

namespace bsp {
// Complete0048D3B0..0048D47A: ECX actual12B iterator(owner0,word4,bit8),
// EDX unconsumed, stacked signed displacement, RET4/EAXsame iterator.
// Zero displacement returns before any pointed-field read. Nonzero retains
// short-circuit null checks, captured backing begin/bit, returning-diagnostic
// reloads, unsigned comparisons, SAR2 position arithmetic and signed borrow.
// Final word RMW precedes bit publication. No added iterator/bounds guard.
void* __fastcall advance_native_checked_bit_iterator_0048d3b0(void* iterator,
    std::uint32_t unused_edx, std::int32_t displacement);

// Complete0048F290..0048F2D6: ECX packed-bit header(count0,begin8,endC),
// EDX unused, stacked12B output, RET4/EAXoutput. Captures begin before
// reporting begin>end; stores output0=0,word,bit0, then null-owner report,
// owner publication, CURRENT count read, and optional advance. Output/receiver
// aliases retain that order; the pointed receiver is read before its null test.
void* __fastcall end_native_checked_bit_vector_0048f290(void* vector,
    std::uint32_t unused_edx, void* output);

// Complete0048F2E0..0048F343: ECX source iterator, EDX unused, stacked
// output/displacement, RET8/EAXoutput. Read owner-test then word/bit before
// reporting owner null. Reload owner AFTER report into a private12B iterator,
// advance it, capture its fields, store output0=0/word/bit, report captured
// owner null, and finally publish captured owner. No struct-copy substitution.
void* __fastcall copy_advance_native_checked_bit_iterator_0048f2e0(
    void* source, std::uint32_t unused_edx, void* output,
    std::int32_t displacement);

// Whole ordinary paths use accepted current SDK00BF6713 policy which may
// return or throw; no new callback/handler/noreturn/exception conversion.
// Reached actual storage must support the exact accesses, including after
// returning diagnostics. No local FH3, snapshots, rollback or owner repair.
// Existing bounded keyboard interfaces/fixtures stay separate. These Source
// interfaces/private cells/dead-EDX normalization do not prove original
// binary ABI, register residue, stack aliases, CRT/SEH/fault/exception/gameplay.
} // namespace bsp
