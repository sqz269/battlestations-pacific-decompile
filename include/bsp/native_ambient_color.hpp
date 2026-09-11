#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native ambient color requires MSVC Win32 x87 assembly.
#endif

namespace bsp {

// Full B84C60..B84C64: ECX actual owner, EAX owner+8, RET. Address only;
// no pointee access, construction, retention or ownership is implied.
const void* __fastcall native_ambient_color_address_00b84c60(
    const void* actual_owner) noexcept;

// Full 4FB850..4FB8EF. Original ECX destination, stack source float4,
// EAX destination, RET4. New fastcall interface additionally borrows the
// actual CE4B48 double scale and mutable0109EEA4 conversion-mode storage.
// Input channels R,G,B,A are consumed in native x87 order. Signed low-EAX
// results clamp to0..255 and publish destination bytes2,1,0,3 in that order.
// Overlap, live conversion-mode reads and partial writes remain visible.
// No float adapter, temporary float4, mode normalization or rollback.
void* __fastcall convert_native_float_rgba_to_argb_004fb850(
    void* actual_destination, const void* actual_source_float4,
    const volatile double* actual_scale_00ce4b48,
    const volatile std::uint32_t* actual_conversion_mode_0109eea4);

} // namespace bsp
