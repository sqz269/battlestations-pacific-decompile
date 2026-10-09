#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native pending registry base cleanup requires MSVC Win32.
#endif

namespace bsp {

// Ordinary 008748F0[17]: clear actual F878CC first, then stamp CE3818 in
// receiver+00. Section+04 is untouched; the clear has no identity guard.
// This new C++ interface borrows the actual mutable publication cell and
// receiver. It creates no owner/cell and does not establish their lifetime.
// The concrete 00412430 Source helper supplies only the profile-store effect;
// the native body has no call. Native entry/FH3/fault behavior, callable table
// delivery and a producer of the genuine publication binding remain separate.
void cleanup_native_pending_registry_base_008748f0(
    void* actual_receiver,
    void* volatile& actual_registry_publication_00f878cc);

} // namespace bsp
