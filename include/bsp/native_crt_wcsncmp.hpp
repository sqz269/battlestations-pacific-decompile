#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native CRT counted wide comparison requires MSVC Win32.
#endif

namespace bsp {

// Whole ordinary-domain Source reconstruction of Original _wcsncmp at
// 00BFDCB7..00BFDCEB (53 bytes, 26 instructions). This is a new typed API over
// little-endian unsigned WORD16 code units, with no Unicode/locale semantics.
// Count zero admits any/null pointers and returns zero without payload reads.
// Otherwise every actually reached WORD read must be determinate, readable,
// stable storage wholly within a genuine live allocation. An early stop does
// not require the unused count-sized extent. Reached advances must not wrap
// and must stay within the same allocation or its endpoint; never read one-past.
// Unaligned raw storage and stable identical/overlapping read-only aliases are
// admitted. No typed wchar/WORD ownership, alignment, padding or signed cap.
//
// Decrement before preliminary reads. LHS zero skips preliminary RHS; only
// equal nonzero pairs advance. Every positive exit freshly reads LHS then RHS,
// returning the exact widened unsigned16 difference, in [-65535,65535].
// Original mutable count-home/stack alias protocols, faults, mutation,
// concurrency, MMIO and lifetime changes are outside this ordinary domain.
// No Original physical ABI/home/register/flags/segment/EH/runtime proof.
// Evidence and remaining integration checks: docs/NATIVE_CRT_WCSNCMP.md.
int __cdecl compare_native_crt_wide_prefix_00bfdcb7(
    const void* actual_left, const void* actual_right, unsigned int count);

} // namespace bsp
