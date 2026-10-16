#pragma once

#include "bsp/native_crt_pointer_decode.hpp"

namespace bsp {

// Actual Win32 TlsSetValue IAT binding (BOOL __stdcall(DWORD, LPVOID)).
// This reference denotes current CE20B4, not an injectable generic provider.
using NativeCrtActualTlsSetValue =
    std::int32_t (__stdcall*)(std::uint32_t, void*);

// Whole C05070[50], a provisional descriptive name. Native has no arguments,
// plain RET and a raw pointer EAX; this Source API adds three borrowed inputs
// (cdecl caller-pop12). Reuse the SAME decode context's actual E15B00 word
// and current CE20BC IAT; do not supply duplicate TLS-index/getter shadows.
// The other references denote actual encoded getter109DE38 and current
// TlsSetValue IAT CE20B4. Binding identities remain valid/stable; values remain
// current. All TLS, encoded-pointer and module-gate services have real owners.
//
// Capture index before getter IAT. Nonnull cached pointer skips every encoded,
// decoder and setter access. On a miss, decode the late encoded word through
// actual C04FDE, then capture CURRENT index before the late setter IAT call.
// Ignore setter BOOL and return captured decoded pointer even null. Native
// discards C04FDE's outgoing argument-word writeback; no caller-variable store
// or LastError repair, default, guard, retry, cleanup or exception translation.
//
// Supplied-context Source only: module-gate errno/invalid services, real CRT
// bootstrap/PTD/locale/TLS ownership remain open. Added references/private
// frame, registers/flags, original fault/SEH/binary ABI and game are unproved.
void* __cdecl select_native_crt_ptd_getter_00c05070(
    const NativeCrtPointerDecodeContext& actual_decode_context,
    const volatile std::uint32_t& actual_encoded_getter_0109de38,
    NativeCrtActualTlsSetValue const volatile& actual_tls_set_value_iat_00ce20b4);

} // namespace bsp
