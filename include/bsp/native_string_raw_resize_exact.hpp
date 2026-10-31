#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Exact native string resize requires MSVC Win32.
#endif

namespace bsp {
struct NativeStringRawPoolContext;

// Whole 0041DD40 normal body against actual raw8 {LEN+0, DATA+4} storage.
// Capture request; equality returns before DATA, HOME or any provider use.
// Nonzero resize reads the actual preserve BYTE HOME only AFTER allocation.
// Preserve always invokes genuine BF7680, including count0, with the SAME
// actual canonical feature cell. Do not substitute a by-value bool facade.
//
// Caller supplies genuine raw backing, a live reached preserve BYTE HOME,
// actual AA8/AA4/AA0 pool cells and the real current feature-word reference.
// Inherit the getter/deletion binding, pool/CRT, copy extent/DF/CPU and lifetime
// domains. No semantic NativeString, new owner, Frame or type birth is added.
//
// No local EH, refund, rollback, retry, validation or failure cleanup. An escape
// may retain unpublished allocation and changed pool/header/copy debt; caller
// retains those actual domains and an incomplete attempt, without a fabricated
// recoverable-pointer receipt. Ordinary Source void completion is not Native
// EAX, __thiscall/RET8, private-HOME/FH3/SEH/fault or parent/runtime permission.
void resize_native_string_header_0041dd40_exact(
    void* actual_header, NativeStringRawPoolContext& context,
    std::uint32_t request, const void* actual_preserve_byte_home,
    const volatile std::uint32_t& actual_feature_word_0109eea4);
} // namespace bsp
