#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native CRT reverse byte search requires MSVC Win32.
#endif

namespace bsp {

// Complete ordinary-domain Source reconstruction of original _strrchr at
// 00BF87B0..00BF87DC (45 bytes, 23 instructions). New typed interface: the
// native cdecl/flat-ES/entry-DF=0 contract is evidence, not binary ABI identity.
//
// actual_string is genuine live storage, readable through its first NUL.
// The NUL-inclusive extent/count must fit without 32-bit wrap. Arguments and
// storage are stable; every int value is admitted and selects its low byte.
// Stable payload aliases are admitted. The result borrows the same storage.
//
// Read every byte forward through NUL, then convert character, read backward
// from NUL through the last match or base, and reread that final byte. No input
// writes, word overreads, host/provider, locale/TLS or search helper are added.
// No null/fault/concurrency/volatile-MMIO protocol or original stack/register/
// flags/segment/EH/compiler/runtime/game identity is provided by this API.
// Evidence and remaining validation: docs/NATIVE_CRT_STRRCHR.md.
char* __cdecl find_native_crt_last_character_00bf87b0(
    char* actual_string, int character);

} // namespace bsp
