#include "bsp/native_input_device_virtuals.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native input selection arithmetic requires MSVC Win32.
#endif

namespace bsp {
namespace {
std::uint32_t profile(const void* owner) noexcept {
    return *static_cast<const volatile std::uint32_t*>(owner);
}
std::uint8_t scan_any(void* owner, std::uint32_t count, NativeInputDeviceVirtualCalls& calls) {
    for (std::uint32_t code = 0; code != count; ++code) {
        const auto captured = profile(owner);
        if (calls.query_vslot20(owner, captured, code) != 0) return 1;
    }
    return 0;
}

// A9446E..A944D3 / A9AB3E..A9ABA3. In particular this is not an ABS/MAX
// replacement: the two sign branches use different comparison instructions,
// and both negative/unordered arms subtract from the live negative-zero word.
bool greater_magnitude(float value, float previous, const NativeInputSelectionConstants& constants) noexcept {
    const volatile float* const negative_zero = &constants.negative_zero_00d7a208;
    const volatile float* const zero = &constants.zero_00d7a218;
    float magnitude, previous_magnitude;
    unsigned char selected;
    __asm {
        fldz
        movss xmm1, value
        fld value
        fcomip st(0), st(1)
        fstp st(0)
        jbe negative_value
        movss magnitude, xmm1
        jmp previous_value
    negative_value:
        mov eax, negative_zero
        movss xmm0, dword ptr [eax]
        subss xmm0, xmm1
        movss magnitude, xmm0
    previous_value:
        movss xmm0, previous
        mov eax, zero
        comiss xmm0, dword ptr [eax]
        jbe negative_previous
        movss previous_magnitude, xmm0
        jmp compare_magnitudes
    negative_previous:
        mov eax, negative_zero
        movss xmm2, dword ptr [eax]
        subss xmm2, xmm0
        movss previous_magnitude, xmm2
    compare_magnitudes:
        fld previous_magnitude
        fld magnitude
        fcomip st(0), st(1)
        fstp st(0)
        seta selected
    }
    return selected != 0;
}
std::int32_t select_greatest(void* owner, std::uint32_t count,
    NativeInputDeviceVirtualCalls& calls, const NativeInputSelectionConstants& constants) {
    std::int32_t selected = -1;
    float previous = 0.0f;
    for (std::uint32_t code = 0; code != count; ++code) {
        const auto query_profile = profile(owner);
        if (calls.query_vslot20(owner, query_profile, code) == 0) continue;
        const auto value_profile = profile(owner); // query may replace raw+0
        const float value = calls.value_vslot24(owner, value_profile, code);
        if (greater_magnitude(value, previous, constants)) {
            selected = static_cast<std::int32_t>(code);
            previous = value; // original signed value, not the magnitude
        }
    }
    return selected;
}
} // namespace

void reset_native_input_device_00a93e80(void*) noexcept {}
void set_native_input_relative_noop_00a93e90(void*, std::uint32_t, std::uint32_t) noexcept {}
std::uint8_t query_native_input_relative_false_00a93ea0(const void*, std::uint32_t) noexcept { return 0; }

const char* native_keyboard_device_name_00a96360(const void*, const NativeInputDeviceNameLiterals& names) noexcept {
    return names.keyboard_00d5b6fc;
}
const char* native_mouse_device_name_00a9a100(const void*, const NativeInputDeviceNameLiterals& names) noexcept {
    return names.mouse_00d5b8a4;
}
const char* native_gamepad_device_name_00a95be0(const void*, const NativeInputDeviceNameLiterals& names) noexcept {
    return names.gamepad_00d5b6ac;
}

std::uint8_t native_gamepad_has_activity_00a93f30(void* owner, NativeInputDeviceVirtualCalls& calls) {
    return scan_any(owner, 90, calls);
}
std::uint8_t native_gamepad_buttons_active_00a93f60(void* owner, NativeInputDeviceVirtualCalls& calls) {
    return scan_any(owner, 60, calls);
}
std::uint8_t native_keyboard_has_activity_00a95ed0(void* owner, NativeInputDeviceVirtualCalls& calls) {
    return scan_any(owner, 256, calls);
}
std::uint8_t native_mouse_has_activity_00a9aab0(void* owner, NativeInputDeviceVirtualCalls& calls) {
    return scan_any(owner, 17, calls);
}
std::uint8_t native_mouse_buttons_active_00a9aae0(void* owner, NativeInputDeviceVirtualCalls& calls) {
    return scan_any(owner, 8, calls);
}
std::uint8_t native_keyboard_buttons_active_00a95f00(void* owner, NativeInputDeviceVirtualCalls& calls) {
    const auto captured = profile(owner);
    return calls.activity_vslot28(owner, captured);
}
std::int32_t select_native_keyboard_control_00a95f10(void* owner, NativeInputDeviceVirtualCalls& calls) {
    for (std::uint32_t code = 0; code != 256; ++code) {
        const auto captured = profile(owner);
        if (calls.query_vslot20(owner, captured, code) != 0) return static_cast<std::int32_t>(code);
    }
    return -1;
}
std::int32_t select_native_gamepad_control_00a94440(void* owner, NativeInputDeviceVirtualCalls& calls,
    const NativeInputSelectionConstants& constants) {
    return select_greatest(owner, 90, calls, constants);
}
std::int32_t select_native_mouse_control_00a9ab10(void* owner, NativeInputDeviceVirtualCalls& calls,
    const NativeInputSelectionConstants& constants) {
    return select_greatest(owner, 17, calls, constants);
}
__declspec(naked) float __fastcall native_mouse_double_click_seconds_00a9a380(const void*) noexcept {
    // Leave the native load in x87; do not add an SSE round trip.
    __asm {
        fld dword ptr [ecx + 238h]
        ret
    }
}
} // namespace bsp
