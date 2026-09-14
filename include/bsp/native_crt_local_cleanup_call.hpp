#pragma once

namespace bsp {
// Complete C16898[3]: CALL EAX; RET (FF D0 C3). Original entry has no
// stack arguments: EAX is the actual executable cleanup target, EBP is the
// inherited native funclet frame, and ECX/every other register arrive from
// the caller. C0DBC4 supplies ECX=1; C167C9 supplies the scope enclosing level.
// Evidence: accepted CP 1d81dafac62a5e3284c9e3da1a68bc4c53a5af4e,
// docs/NATIVE_CRT_LOCAL_CLEANUP_CU.md and its companion report.
//
// This is a naked Win32 register entry, not an ordinary C++ callback API.
// The caller supplies the actual valid frame, native stack and cleanup body.
// CALL adds its real return word; a balanced cleanup return is required for
// the final RET to resume this entry's caller. No setup, saves, normalization,
// validation, catch or cleanup/FS/cookie/NLG owner is supplied. The actual
// cleanup's register and flag effects propagate without normalization.
// Exceptional/nonlocal exit does not guarantee execution of the final RET.
// Exact primitive bytes do not establish enclosing native-frame, exception,
// caller or gameplay validity; C0DC54 and its providers remain separate.
void __cdecl call_native_crt_cleanup_00c16898();
} // namespace bsp
