#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native string scalar construction requires MSVC Win32 x87 operations.
#endif

#include "bsp/native_string.hpp"

namespace bsp {

// Complete 00444070. Native ECX destination; stack eight-byte source header and
// scalar address; EAX destination; RET8. Destination is actual writable 12 bytes.
// Clears destination+0/+4 before the identity branch, abandoning old storage.
// The concrete raw-pool copy finishes BEFORE loading the four bytes at scalar.
// scalar may alias either header, their data, or destination+8; it is not a
// captured float value. The final x87 FLD/FSTP writes destination+8 even when
// the headers are identical or empty. No local cleanup on construction failure.
// New C++/explicit-context ABI, not a binary replacement. See the packet doc.
void* copy_construct_native_string_with_scalar_00444070(void* actual_destination,
    const void* actual_source_header, const void* actual_scalar,
    NativeStringRawPoolContext& context);

// Complete 004440C0. Native ECX destination; stack 12-byte source storage; EAX
// destination; RET4. Same header construction, then a late x87 load of source+8
// and store to destination+8. Identity still clears the header and executes the
// scalar conversion, including x87 NaN/status/exception effects. No owner
// overlay, scalar snapshot, null guard, or rollback is introduced.
void* copy_construct_native_string_with_scalar_004440c0(void* actual_destination,
    const void* actual_source, NativeStringRawPoolContext& context);

} // namespace bsp
