#pragma once

#include <cstdint>

namespace bsp {

// Whole Original ___ascii_stricmp [00BF7EB7,00BF7EEC), 53B/24 instructions.
// Borrow valid reached raw bytes; read left then right even when left is NUL.
// Fold only unsigned BYTE values A..Z, advance both cursors modulo Win32,
// test folded left==0 before equality, and return their signed difference.
// All BYTE values are intrinsic to this leaf; no SDK/locale/TLS/errno service
// or parent locale-dispatch behavior is supplied. No guards, writes or owners.
// Original is cdecl(two stacked pointers), plain RET/EAX int. This ordinary
// Source interface does not establish private argument-home aliases, fault/SEH,
// Original entry/code/register identity, whole-parent or game compatibility.
std::int32_t compare_native_crt_ascii_strings_00bf7eb7(
    const char* actual_left, const char* actual_right) noexcept;

} // namespace bsp
