#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw GUI named-value copy requires MSVC Win32.
#endif

namespace bsp {
struct NativeStringRawPoolContext;

// Ordinary caller-retained backing for the reached PUSH1 DWORD. No initializer:
// self-copy leaves its preimage untouched. Keep its actual address through the
// resize call and any incomplete attempt; the callee reads its BYTE after the
// real pool allocation. This is neither an owner nor a completion/origin token.
struct NativeGuiNamedValueCopyStorageFrame {
    std::uint32_t preserve_home;
};
static_assert(sizeof(NativeGuiNamedValueCopyStorageFrame) == 4);

// Whole normal AB4690[72B/28ops]: actual12B {LEN,DATA,opaque DWORD} copy ctor.
// Capture identities/equality, clear destination LEN/DATA even for self-copy,
// then nonself exact41DD40 with the actual preserve HOME. CURRENT sourceLEN
// gates genuine BF7680, including CURRENT destination count0; source+8 is read
// only after that copy. Return captured destination; no extra EOF or release.
//
// Caller establishes genuine ordinary raw12B backing/subobject lifetimes and
// preserves supported payload/header/data aliases. Retain both payloads, their
// live buffers, this separate private HOME, raw AA8/AA4/AA0 pool domains and the
// SAME canonical live0109EEA4 feature reference through callbacks/incomplete
// disposition. Inherit exact resize/getter/new-handler/deletion and BF valid
// extents/DF0/CPU domains; active private/provider-frame copying is excluded by
// the existing BF contract. No semantic NativeString or storage owner is born.
//
// No local EH, cleanup, retry, rollback, guard or Native-origin permission.
// An escape leaves the reached raw effects/debt for caller-retained disposition.
// New ordinary C++ ABI, not original ECX/RET4/private-stack/flags/fault/FH3 or
// runtime/parent/queue/activation proof. Old entry copies remain unchanged.
void* copy_construct_native_gui_named_value_storage_00ab4690(
    void* actual_destination12, const void* actual_source12,
    NativeGuiNamedValueCopyStorageFrame&, NativeStringRawPoolContext&,
    const volatile std::uint32_t& actual_feature_word_0109eea4);
} // namespace bsp
