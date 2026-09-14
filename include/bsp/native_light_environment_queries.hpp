#pragma once

#include <cstdint>

namespace bsp {

// Complete 00B7AA20 and 00B7AA30. ECX is the actual light-environment
// storage address. EAX is an interior borrowed ADDRESS, not a loaded pointer
// or color value. LEA does not dereference ECX, even when it is zero.
const void* __fastcall get_native_light_environment_ambient_00b7aa20(
    const void* actual_environment) noexcept;
const void* __fastcall get_native_light_environment_mode3_ambient_00b7aa30(
    const void* actual_environment) noexcept;

// Complete 00B7AA40. Original ECX is the actual environment, the one stack
// DWORD is an unchecked index, EAX=ECX+38h+(index<<4) with 32-bit wrapping,
// and RET4 consumes that index. The source fastcall explicitly reserves EDX
// as an unused second argument so its third argument occupies the original
// stack slot. No color read, bounds check, null repair, or owner projection.
const void* __fastcall get_native_light_environment_cube_face_00b7aa40(
    const void* actual_environment, void* unused_edx,
    std::uint32_t index) noexcept;

} // namespace bsp
