#pragma once

#include <cstdint>

namespace bsp {

// Each caller reads actual_device+0 at the native call point. The provider
// resolves THAT captured profile, using the same raw allocation as receiver.
// It must not cast raw storage to InputDevice or silently accept unknown slots.
class NativeInputDeviceVirtualCalls {
public:
    virtual ~NativeInputDeviceVirtualCalls() = default;
    virtual std::uint8_t query_vslot20(void* actual_device,
        std::uint32_t captured_profile, std::uint32_t code) = 0;
    virtual float value_vslot24(void* actual_device,
        std::uint32_t captured_profile, std::uint32_t code) = 0;
    virtual std::uint8_t activity_vslot28(void* actual_device,
        std::uint32_t captured_profile) = 0;
};

struct NativeInputSelectionConstants {
    const volatile float& negative_zero_00d7a208;
    const volatile float& zero_00d7a218;
};
// Canonical borrowed literal addresses, not owned/generated NativeStrings.
// Native bytes are respectively "Keyboard", "Mouse", "GameController".
struct NativeInputDeviceNameLiterals {
    const char* keyboard_00d5b6fc;
    const char* mouse_00d5b8a4;
    const char* gamepad_00d5b6ac;
};

// Native ECX unconsumed. A93E80 RET; A93E90 RET8 with both words ignored;
// A93EA0 XOR AL,AL; RET4. No state or native reference-count changes.
void reset_native_input_device_00a93e80(void*) noexcept;
void set_native_input_relative_noop_00a93e90(void*, std::uint32_t, std::uint32_t) noexcept;
std::uint8_t query_native_input_relative_false_00a93ea0(const void*, std::uint32_t) noexcept;

// Native ECX unconsumed, EAX original literal address, RET. Added literal
// binding supplies the caller's canonical source address and changes the ABI.
const char* native_keyboard_device_name_00a96360(const void*, const NativeInputDeviceNameLiterals&) noexcept;
const char* native_mouse_device_name_00a9a100(const void*, const NativeInputDeviceNameLiterals&) noexcept;
const char* native_gamepad_device_name_00a95be0(const void*, const NativeInputDeviceNameLiterals&) noexcept;

// ECX actual device, no native stack args, RET/AL Boolean. Each queried code
// rereads the raw profile. No copied state arrays or cached derived identity.
std::uint8_t native_gamepad_has_activity_00a93f30(void*, NativeInputDeviceVirtualCalls&); //0..89
std::uint8_t native_gamepad_buttons_active_00a93f60(void*, NativeInputDeviceVirtualCalls&); //0..59
std::uint8_t native_keyboard_has_activity_00a95ed0(void*, NativeInputDeviceVirtualCalls&); //0..255
std::uint8_t native_mouse_has_activity_00a9aab0(void*, NativeInputDeviceVirtualCalls&); //0..16
std::uint8_t native_mouse_buttons_active_00a9aae0(void*, NativeInputDeviceVirtualCalls&); //0..7
// Native tail JMP current vslot28 with same ECX/no args. Preserve its AL result.
std::uint8_t native_keyboard_buttons_active_00a95f00(void*, NativeInputDeviceVirtualCalls&);

// ECX actual device, RET/EAX code or-1. Keyboard returns first down code;
// gamepad/mouse retain first strict greatest magnitude among queried-down codes.
// Selection reloads profile again between query20 and value24, spills ST0 to
// binary32, and retains the recovered x87/SSE comparison/subtraction sequence.
std::int32_t select_native_keyboard_control_00a95f10(void*, NativeInputDeviceVirtualCalls&);
std::int32_t select_native_gamepad_control_00a94440(void*, NativeInputDeviceVirtualCalls&,
    const NativeInputSelectionConstants&);
std::int32_t select_native_mouse_control_00a9ab10(void*, NativeInputDeviceVirtualCalls&,
    const NativeInputSelectionConstants&);

// FLD actual mouse+238; RET/ST0. Only D5B8B0's slot38 has this signature.
// Keyboard D5B904 ends at slot34; D5B670/D5BB48/D5B7F0 slot38 is force output.
float __fastcall native_mouse_double_click_seconds_00a9a380(const void*) noexcept;

// Complete source control flow with explicit raw virtual providers. Original
// register upper bits, stack layout/FH3/SEH and drop-in vtable compatibility
// are not claimed. No lifetime manager, SDK owner, GUID vector or force-request
// producer is introduced. Existing typed counterparts remain separate.
} // namespace bsp
