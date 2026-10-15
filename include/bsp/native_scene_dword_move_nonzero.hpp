#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Nonzero raw DWORD range movement requires MSVC Win32.
#endif

namespace bsp {

// Complete 00468F60..00468F8F: native stdcall(first,last,destination), RET0Ch,
// incoming ECX/EDX unused, ESI preserved. Capture last, first, destination;
// subtract addresses modulo32, reproduce SAR2 bits, multiply by4 modulo32,
// and capture destination+bytes BEFORE the real CRT can report, return or throw.
// For EVERY nonzero shifted word count, including negative bits, perform actual
// current SDK memmove_s(destination,bytes,first,bytes) and ignore its status.
// Zero shifted count skips CRT but still returns the captured wrapped result.
void* __stdcall move_native_scene_dword_range_00468f60(
    const void* first, const void* last, void* destination);

// Ordinary Source raw pointer values/current SDK boundary; no alignment/order/
// positive-count guard, callback, pool action, cleanup or nominal ownership is
// added. Reached backing must satisfy the actual current SDK service. Source/
// native private-frame and argument-home aliases, original static CRT errno/
// handler/global/exception/fault/SEH/instruction/ABI and game identity are not
// established. The calling-convention keyword alone is not binary ABI proof.

} // namespace bsp
