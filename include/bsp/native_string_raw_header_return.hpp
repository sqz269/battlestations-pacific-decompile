#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Exact raw string-header return requires MSVC Win32.
#endif

namespace bsp {

struct NativeStringRawPoolContext;

// Whole 0041DD20, ordinary potentially-throwing normal-call capability only.
// CURRENT DATA+4 FIRST; NULL skips LEN/cell values/getter/return. Nonnull:
// capture CURRENT LEN+0 and LEN+1 DWORD wrap before actual AA8/AA0 getter,
// then real BD1510 with captured block/count and SAME live AA4 reference.
// No post-call header read/store/reset or separate header allocation free.
//
// Caller retains actual reached raw8 representations/backing, genuine block
// and size-class/heap ownership, actual pool/manager/cells/code/allocator,
// synchronization and all required alias/lifetime/partial-failure custody.
// Getter C++ escape propagates; return-provider noexcept/CRT/gate domains
// remain inherited. No owner/Frame/type birth/guard/cleanup/refund/retry,
// healthy receipt or Source/Native ABI/flags/private-home/fault equivalence.
// This normal helper does NOT choose parent selected-scope, secondary-escape
// or destructor-termination policy; keycopy/keystring/builder/VM remain gated.
void return_native_string_header_buffer_0041dd20_exact(
    const void* actual_header8, NativeStringRawPoolContext& actual_strings);

} // namespace bsp
