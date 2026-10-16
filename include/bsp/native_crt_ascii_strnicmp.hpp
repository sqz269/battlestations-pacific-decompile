#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error This actual-storage reconstruction requires MSVC Win32.
#endif

namespace bsp {

// Complete Original 00C15700 ASCII comparison leaf, through a new interface.
// Native: cdecl(left pointer, right pointer, unsigned count), plain RET,
// EAX exactly -1, 0 or +1. This is not its binary calling convention.
//
// actual_parameters12 binds native left/right/count DWORD cells at +0/+4/+8.
// Read count first; zero skips both pointer cells and all payload. Nonzero
// captures the pointer words once, then reads each left BYTE before its right
// BYTE, including the right read when the left byte is NUL. Only reached
// storage must be valid; there is no null/count/alignment validation.
//
// The binding address stays stable. New C++ metadata/private-frame backing is
// disjoint from reached cells/payload. Payload may alias actual cells. No owner,
// locale/TLS/SDK service, private-frame/fault/flags or game identity is implied.
std::int32_t compare_native_crt_ascii_prefix_00c15700(
    const void* actual_parameters12);

} // namespace bsp
