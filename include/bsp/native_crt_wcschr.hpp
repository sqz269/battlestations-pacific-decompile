#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native CRT wide character search requires MSVC Win32.
#endif

namespace bsp {

// Whole ordinary-domain Source reconstruction of Original _wcschr at
// 00BFD6C1..00BFD6E2 (34 bytes, 14 instructions). New typed API; every int
// target selects its unsigned low16 bits. Code units are little-endian WORD16,
// without Unicode/locale/case semantics. The signature is potentially throwing.
//
// Require a finite reached first match/NUL. Every actually reached WORD read,
// including the mandatory NUL reread, lies wholly in genuine live stable
// determinate readable byte storage in one allocation. A nonzero match needs
// no unreached later-NUL extent. Null/invalid input is outside this domain.
// Admit unaligned raw storage, prefixes/interior regions and read-only stable
// aliases. Reached advances do not wrap and stay within or one past the same
// allocation; never read one-past. No typed wchar/WORD ownership or padding.
//
// A nonzero match returns its genuine borrowed pointer without rereading.
// NUL always causes one fresh same-WORD reread, including target zero, before
// pointer/null selection. The routine never writes payload or transfers owner
// lifetime. Mutation/concurrency, faults, MMIO/volatile observation and lifetime
// changes are excluded. No Original physical ABI/home/register/flags/segment/
// stack/EH/instruction/runtime identity is supplied. See docs/NATIVE_CRT_WCSCHR.md.
void* __cdecl find_native_crt_wide_character_00bfd6c1(
    void* actual_string, int character);

} // namespace bsp
