#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error This interface requires MSVC Win32 and its addressed WORD16 load primitive.
#endif

namespace bsp {

// Whole ordinary raw-storage interface for Original _wcscspn at 00C0382C.
// Every reached WORD needs two genuine stable live determinate readable bytes;
// raw unaligned/interior/read-only/overlapping inputs are allowed. Advances are
// nonwrapping within or one-past their actual allocation, never reading one-past.
// Initial source NUL needs no control storage and permits a null control pointer.
// A finite reached stop is required; unvisited suffixes need no terminator/padding.
// Returns exact native SUB/SAR bits: n | ((n & 0x40000000U) << 1), where n counts
// traversed source code units. No signed-distance cap or wide-object ownership.
// Stable ordinary caller-stack storage is allowed; callee-private frame/home,
// fault/MMIO/concurrency/lifetime/segment protocols and physical ABI are excluded.
// New default-potentially-throwing API; no Unicode/locale or game validation.
unsigned int __cdecl count_native_crt_wide_prefix_excluding_00c0382c(
    const void* actual_source, const void* actual_control);

} // namespace bsp
