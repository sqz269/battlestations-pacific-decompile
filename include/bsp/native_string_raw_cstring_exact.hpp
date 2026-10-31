#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Exact native CString construction requires MSVC Win32.
#endif

namespace bsp {
struct NativeStringRawPoolContext;

// Whole 0041E870 normal constructor transport over actual writable raw8 backing.
// Capture header/input before LEN0 then DATA0; only then scan actual bytes through
// NUL. A private DWORD1 HOME feeds genuine exact resize. After its normal return,
// CURRENT DATA gates CURRENT LEN+1 and genuine BF7680, including wrapped count0.
// The SAME actual feature-word reference reaches both providers without preload.
//
// Caller supplies established live raw8 backing, genuine reached CString/alias
// extents, SAME actual pool cells/manager/allocator/code, feature-cell/CPU/FP/DF,
// private-HOME and lower-provider lifetime/error domains. No birth or adoption,
// semantic header, owner, Frame, guard or validation is added.
//
// No local EH/free/refund/rollback/retry. On escape caller retains unpublished
// allocation, pool/header/copy and incomplete-attempt debt. Its own cleanup can
// arm only after actual normal return. Returning captured header identity through
// ordinary Source is not Native __thiscall/RET4, FH3/SEH/fault or runtime proof.
void* construct_native_string_header_0041e870_exact(
    void* actual_header, const void* actual_cstring,
    NativeStringRawPoolContext& context,
    const volatile std::uint32_t& actual_feature_word_0109eea4);
} // namespace bsp
