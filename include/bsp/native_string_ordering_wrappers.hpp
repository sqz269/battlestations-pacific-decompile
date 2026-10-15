#pragma once

namespace bsp {

// Complete 00443CB0..00443CFC: native ECX left actual8h string header,
// stack right header, RET4, AL boolean (upper EAX is not normalized).
// Empty means stored length+0 == 0, independent of data+4. For two nonempty
// headers use current CRT _stricmp, reading right data before left data.
// Shares the already reconstructed complete 00443D00 predicate, whose native
// argument ABI differs. No null-header guard, byte copy or length-bounded compare.
bool less_native_string_headers_00443cb0(
    const void* actual_left_header, const void* actual_right_header);

// Complete 0076EAA0..0076EAF6: native ECX left/EDX right actual16h records,
// RET, EAX0/1. Compare the native8h string prefix first; when not less,
// compare current prefixes in reverse, then read the unsigned scalar suffix
// high DWORD+Ch before low DWORD+8h. Low words are read only on high equality.
// Do not snapshot records across either CRT comparison or infer a LAN/host
// identity for the suffix. Legacy MpSeg43Hooks projection remains separate.
// These ordinary C++ bool interfaces do not reproduce the original ABI.
bool less_native_string_scalar_keys_0076eaa0(
    const void* actual_left_record, const void* actual_right_record);

} // namespace bsp
