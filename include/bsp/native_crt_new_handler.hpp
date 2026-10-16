#pragma once

#include "bsp/native_crt_pointer_decode.hpp"

namespace bsp {

// Whole C055B1[34], original library identity __callnewh. The descriptive
// Source name is provisional. Native cdecl(size) uses caller-pop4/plainRET
// and returns exact 0/1; Source adds two borrowed references, caller-pop12.
// Reuse the SAME existing decode context and actual current encoded handler
// cell109DE44. No handler/global/TLS/heap owner or default is supplied.
//
// Read encoded word, capture actual C04FDE return; discard its outgoing first
// argument-word writeback. Null decoder result skips size read/handler call.
// Nonnull invokes the actual installed int32 __cdecl(uint32) handler and
// normalizes every nonzero result to1. No std::new_handler(void) policy.
//
// The late volatile load is of this NEW Source parameter's own word. It is
// not an Original stack-frame reader or an outgoing-slot alias contract.
// References/binding identities remain valid and stable; their values retain
// real supplied owners. Inherited module-gate services/runtime stay open.
// No catch, cleanup, errno, lock, retry, LastError wrapper or noexcept adapter.
// Original private arguments/registers/flags/fault PCs/FH3/SEH/CRT/TLS/handler
// ownership/binary ABI and game behavior are not established.
std::int32_t __cdecl invoke_native_crt_new_handler_00c055b1(
    std::uint32_t requested_bytes,
    const NativeCrtPointerDecodeContext& actual_decode_context,
    const volatile std::uint32_t& actual_encoded_new_handler_0109de44);

} // namespace bsp
