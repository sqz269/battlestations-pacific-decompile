#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error This literal native entry requires MSVC Win32.
#endif

namespace bsp {

// Whole native 00438E40..00438E79: actual borrowed text in ECX, no stack
// arguments, full EAX result and RET 0. Null returns null without providers.
// Otherwise the source must remain a live, readable, NUL-terminated byte
// object whose length + 1 and address range do not wrap. On normal success
// the result owns that many bytes in the current canonical allocation domain;
// release it once with singleton_lifetime_free after the final observation.
// The two genuine calls bind current canonical allocation and standard memcpy.
// Original private CRT/EH, failure unwind and class ownership are not admitted.
// This entry does not change the older plain-CDECL duplicate_00438e40 API.
char* __fastcall duplicate_native_string_00438e40(const char* actual_text_ecx);

}  // namespace bsp
