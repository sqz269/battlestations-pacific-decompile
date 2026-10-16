#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual DWORD-vector header Append requires MSVC Win32.
#endif

namespace bsp {

// Whole 00698980: 131 bytes/53 current starts. Preserve the established library
// identity LIBSTL_CheckedDwordVectorRows_Append_00698980. Native ECX receiver,
// incoming EDX unused, one stacked fill-header pointer, RET4 on both paths.
// Source void excludes incidental native EAX and the slow private-output address.
void __fastcall append_native_checked_dword_vector_headers_00698980(
    void* receiver, std::uint32_t unused_edx, const void* fill_header);

// Capture initial begin +4; only if nonzero read current end +8 and capacity +C
// for wrapped SAR4 distances and the unsigned spare-capacity comparison. Spare
// capacity calls actual counted fill at a separately captured CURRENT end,
// then publishes captured end+10h even if child aliases changed current end.
// No output-null guard is added: the actual child handles its null output.
//
// The native fast call passes source, receiver bits, duplicate source bits and
// a DWORD whose low byte alone was initialized. Core/catch/action never consume
// the last two words. Source passes canonical unused tag0, not the unspecified
// native high bits; owner bits remain receiver bits as observed.
//
// Slow path captures current end before the initial-begin>end diagnostic. A
// returning report retains that captured end. Actual InsertOne receives the
// receiver, a private eight-byte output pair, receiver owner bits, that position
// and fill header. Ignore its output/result; add no catch, cleanup or rollback.
//
// All public raw DWORD reads/stores are volatile; address arithmetic wraps.
// Reached accesses and child loops must be valid/finite in accepted current
// allocation/CRT domains. Opaque receiver+0 is not directly accessed; public
// child/output aliases retain their actual effects. New Source private argument,
// scratch and register/flag identities differ; native private-stack aliases,
// original CRT/heap/handler/FH3/SEH/fault/RTTI, binary ABI and game equivalence
// remain unproved. No noexcept bridge. Existing bounded storage/Hook projections,
// their fixtures and library names remain separate; Source names are hypotheses.

} // namespace bsp
