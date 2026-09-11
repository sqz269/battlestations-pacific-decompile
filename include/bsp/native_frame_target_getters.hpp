#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual frame target getters require MSVC Win32.
#endif

namespace bsp {

// Complete B1F6D0: original ECX owner, stack slot, EAX borrowed surface, RET4.
// New two-argument fastcall passes the slot in EDX and uses plain RET. Reads
// owner+8+4*slot with wrapped x86 address arithmetic and no range/ref checks.
void* __fastcall native_frame_targets_get_color_surface_00b1f6d0(
    const void* actual_targets, std::uint32_t slot) noexcept;

// Complete four-byte ECX-input leaves. The depth result is borrowed; sRGB
// exposes the exact +3C byte through AL, including non-Boolean values.
void* __fastcall native_frame_targets_get_depth_surface_00b1f6e0(
    const void* actual_targets) noexcept;
std::uint8_t __fastcall native_frame_targets_get_srgb_write_byte_00b1f710(
    const void* actual_targets) noexcept;

} // namespace bsp
