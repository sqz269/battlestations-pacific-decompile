#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native surface dimension leaves require MSVC Win32.
#endif

#include <cstdint>

namespace bsp {

// Complete four-byte D619A0 surface leaves. The original callable ABI is ECX
// actual surface, EAX current unsigned DWORD, plain RET. This raw borrowed
// pointer is the actual NativeSurfaceOwnerStorage address, not a COM surface,
// semantic size, or host owner view. No profile/null/bounds repair occurs.
std::uint32_t __fastcall get_native_surface_width_00b3cd10(
    const void* actual_surface) noexcept;
std::uint32_t __fastcall get_native_surface_height_00b3cd20(
    const void* actual_surface) noexcept;

} // namespace bsp
