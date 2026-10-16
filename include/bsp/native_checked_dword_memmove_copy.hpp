#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Checked DWORD memmove copy requires MSVC Win32.
#endif

namespace bsp {

// Complete ordinary 00440C70..00440C9F (48B/18 instructions).
// Native: three stacked words {first,last,destination}, RET0Ch, EAX captured
// destination + ((SAR2(wrapped last-first)) << 2). ECX/EDX inputs are unused.
// Preserve the established STL_Copy_00440c70 library name; this label is a
// descriptive Source hypothesis. Zero SAR result alone skips the SDK call;
// nonzero negative counts still call with wrapped unsigned bytes. No guard,
// ownership, count bridge or rollback is added. Return is captured before call.
// Direct current-SDK memmove_s is the explicitly accepted provider boundary;
// Original BF67A7 CRT/validation/fault identity is not reconstructed here.
// New private argument/frame/register/flag aliases, Original binary/FH3/SEH/
// heap/runtime/gameplay equivalence remain unproved. No noexcept bridge.
void* __stdcall copy_native_checked_dword_memmove_00440c70(
    const void* first, const void* last, void* destination);

} // namespace bsp
