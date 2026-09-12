#pragma once
#include "bsp/native_string.hpp"
#include <cstdint>

namespace bsp {
// Actual mouse23Ch storage. Getters: ECX receiver, EAX DWORD, RET.
// Setters: ECX receiver, one DWORD stack argument, RET4; raw bits preserved.
// These are D5B8B0 slots3C/40/44/48/4C/50, after the ST0 getter at38.
std::uint32_t native_mouse_accumulated_x_00a99f10(const void*) noexcept;
std::uint32_t native_mouse_accumulated_y_00a99f20(const void*) noexcept;
std::uint32_t native_mouse_accumulated_z_00a99f30(const void*) noexcept;
void set_native_mouse_accumulated_x_00a99f40(void*, std::uint32_t) noexcept;
void set_native_mouse_accumulated_y_00a99f50(void*, std::uint32_t) noexcept;
void set_native_mouse_accumulated_z_00a99f60(void*, std::uint32_t) noexcept;

// A99710, D5B7F0 slot3C: ECX actualB48h; actual8h output/code stack;
// EAX output, RET8. Constructs output empty before reading raw binding29C.
// Uses actual1Ch metadata records from +290, with no typed device companion,
// copied binding vector, or new index guard. Caller supplies valid storage and
// native nonoverlapping memcpy domains. Original kind reload after allocation,
// output/name aliasing and suffix-then-result C++ exception cleanup are retained.
// On failure, result storage is freed but its header stays unchanged and must
// not be freed again. Added storage changes the ABI; FH3/SEH is not reproduced.
NativeString& native_joystick_control_name_00a99710(void* actual_device,
    NativeString& actual_output, std::uint32_t code, NativeStringStorage&);
} // namespace bsp
