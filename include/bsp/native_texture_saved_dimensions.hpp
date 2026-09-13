#pragma once
#include <cstdint>

namespace bsp {

// Complete four-byte native getters. Original ECX is the actual 2D texture
// owner, EAX is a raw DWORD result, no stack arguments, plain RET. D61948
// slots +48/+4C select these saved dimensions, produced by B3F930 at +34/+38.
// The separate descriptor outputs at +28/+2C are different fields.
// These borrowed-pointer C++ interfaces add no owner, COM query or fallback.
std::uint32_t __fastcall get_native_texture_saved_width_00b3ce70(
    const void* actual_owner) noexcept;
std::uint32_t __fastcall get_native_texture_saved_height_00b3ce80(
    const void* actual_owner) noexcept;

} // namespace bsp
