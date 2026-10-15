#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual checked DWORD vector wrappers require MSVC Win32.
#endif

namespace bsp {
// Whole 00697DC0, 137 bytes / 56 instructions. Native ECX=actual header,
// incoming EDX ignored; FOUR stacked words are output, iterator owner,
// position and value ADDRESS. RET10h; EAX returns the original output pointer.
// Header opaque/begin/end/capacity offsets are 0/4/8/C, elements are DWORDs.
// Keep initial begin/index and new begin/result captured across RETURNING
// diagnostics; specified later header reads stay current. Reuse the accepted
// actual 00697990 with count1. Write output+4 position BEFORE output+0 owner.
void* __fastcall insert_native_dword_vector_one_00697dc0(
    void* actual_header, std::uint32_t unused_edx, void* output,
    void* iterator_owner, void* position, const void* value_address);

// Whole 00697F40, 131 bytes / 53 instructions. Native ECX=actual header,
// incoming EDX ignored; one stacked value ADDRESS; both exits RET4. Source
// result is void; original ambient EAX is not a semantic result. Spare space
// uses actual 006970D0 count1, then publishes CAPTURED end+4. Otherwise retain
// captured end through a returning diagnostic and use InsertOne/private8B out.
void __fastcall append_native_checked_dword_vector_00697f40(
    void* actual_header, std::uint32_t unused_edx, const void* value_address);

// No value snapshot, overlay, provider, additional guard, rollback or local
// cleanup. Raw DWORD MOVs, SAR2 and modulo-2^32 arithmetic preserve progressive
// aliases and output-store order. Reached storage must permit the specified
// accesses and accepted child termination; header/output must remain writable.
// The current CRT diagnostic may return, throw or terminate under its selected
// policy; accepted insertion allocation/owning C++ exception limits apply.
// A dead fill tag uses 0 rather than reading unspecified native high bytes.
// Private Source scratch/argument aliases, original physical frame, ambient
// registers/flags, native fault/SEH/FH3/CRT/heap identity and binary/game runtime
// equivalence are unproved. Matching physical formal shapes is not ABI proof.
// Existing library identities and older qualified fragments remain separate;
// new descriptive Source names are hypotheses.
} // namespace bsp
