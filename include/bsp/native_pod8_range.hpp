#pragma once

#include <cstdint>

namespace bsp {
// Complete 00419960..0041998B. Original physical ABI: ECX first, EDX last,
// four stack DWORDs destination,ignored0,ignored1,tag; RET10h. EAX is the
// advanced destination. The three metadata words are never read. Source uses
// that exact six-word fastcall binding; this is an integer-word copy, not x87.
// Each iteration checks the CURRENT destination for zero, reads word0, writes
// word0, then rereads word1 and writes word1. Both cursors advance by native
// DWORD ADD8 even when destination is zero. Forward overlap/aliasing is kept.
// Equal endpoints do not access pointed storage. The +8/equality schedule must
// terminate and every access actually reached must be admitted; no guard,
// allocation, endpoint subtraction or C++ pointer arithmetic is substituted.
void* __fastcall copy_native_pod8_range_00419960(
    const void* first, const void* last, void* destination,
    std::uint32_t ignored0, std::uint32_t ignored1, std::uint32_t tag) noexcept;

// Complete 0041C7E0..0041C80B. Original physical ABI: ECX destination, EDX
// unsigned32 count; stack value,ignored0,ignored1,tag; RET10h. Same null test
// and word0-read/write-before-word1-read/write order as the copy loop; value
// is reread each iteration and can alias either destination word. Count0
// touches no pointed storage and leaves incoming EAX unchanged. Nonzero count
// loads value into EAX even when all stores are skipped; EDX ends zero, ECX
// advances by count*8 modulo2^32. No semantic return value is established.
void __fastcall fill_n_native_pod8_range_0041c7e0(
    void* destination, std::uint32_t count, const void* value,
    std::uint32_t ignored0, std::uint32_t ignored1, std::uint32_t tag) noexcept;

// Complete 0041E3C0..0041E3F1 and 0041F4E0..0041F504. Original wrappers:
// ECX is a metadata preimage, three stack words as below, RET0Ch. EDX is NOT
// an original argument. The explicit Source fastcall adapter consumes an
// unused EDX placeholder to bind that original register/stack layout. Actual
// callers can instead set ECX and push only the original three stack words.
// Each wrapper copies incoming ECX to a local and zeroes ONLY its low byte.
// The complete original four-word kernel call is retained. On return ECX is
// the masked preimage, not a restored unmodified ECX. ESI/EDI are preserved.
// Fill returns first+count*8 modulo2^32, including count0; copy returns the
// actual copy-kernel EAX. No value is cached or interpreted as a private pair.
void* __fastcall fill_n_native_pod8_dispatch_0041e3c0(
    std::uint32_t ecx_preimage, std::uint32_t unused_edx,
    void* first, std::uint32_t count, const void* value) noexcept;

void* __fastcall copy_native_pod8_dispatch_0041f4e0(
    std::uint32_t ecx_preimage, std::uint32_t unused_edx,
    const void* first, const void* last, void* destination) noexcept;

// Descriptive Source names are hypotheses. These exact Win32 bodies and the
// explicit binding do not establish original C++ template/return types, an
// installed replacement, Native runtime differential, or game validation.
} // namespace bsp
