#pragma once
#include "bsp/joystick_input.hpp"

namespace bsp {
// 00A99710: ECX=device, output NativeString*/code stack slots, RET8, EAX=output.
// Constructs output as empty before reading the binding; overwriting a live
// output follows native constructor semantics. Kind0 stays empty. Otherwise
// copy the selected object name and append the recovered directional suffix.
// Invalid native binding/object indices become explicit host errors after the
// initial output construction. On exception, temporary then output storage is
// freed by native0041DD20 semantics: the output header is deliberately unchanged.
NativeString& joystick_control_name_00a99710(const JoystickInputDevice&, NativeString& output,
    std::uint32_t code, NativeStringStorage& = crt_string_storage());
} // namespace bsp
