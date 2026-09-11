#include "bsp/input_device_state.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define DIRECTINPUT_VERSION 0x0800
#include <windows.h>
#include <dinput.h>

#include <cstddef>
#include <stdexcept>

namespace bsp {
namespace {

static_assert(sizeof(MouseInputSample) == sizeof(DIMOUSESTATE2));
static_assert(sizeof(MouseInputSample) == 20);
static_assert(offsetof(MouseInputSample, buttons) == offsetof(DIMOUSESTATE2, rgbButtons));

float scale_mouse_axis(float axis, float scale) {
    const double divisor = 100.0; // 00d7a220
    float result;
    __asm {
        fld scale
        fdiv divisor
        fmul axis
        fstp result
    }
    return result;
}

float integer_mouse_axis(std::int32_t axis, bool negate_integer) {
    // CVTSI2SS uses MXCSR rounding. Wheel NEG happens in integer width before
    // conversion, so INT_MIN remains INT_MIN (not positive 2147483648).
    float result;
    __asm {
        mov eax,axis
        cmp negate_integer,0
        je convert_axis
        neg eax
    convert_axis:
        cvtsi2ss xmm0,eax
        movss result,xmm0
    }
    return result;
}

float invert_mouse_axis(float axis) {
    const float negative_zero = -0.0f; // 00d7a208
    float result;
    __asm {
        movss xmm0,negative_zero
        subss xmm0,axis
        movss result,xmm0
    }
    return result;
}

float milliseconds_to_seconds(std::uint32_t milliseconds) {
    const float unsigned_bias = 4294967296.0f; // 00ce3978
    const double divisor = 1000.0; // 00ce47a0
    float result;
    __asm {
        fild milliseconds
        cmp milliseconds,0
        jge divide_milliseconds
        fadd unsigned_bias
    divide_milliseconds:
        fdiv divisor
        fstp result
    }
    return result;
}

InputStateDevice& query_device(InputDevice& device) {
    auto* result = dynamic_cast<InputStateDevice*>(&device);
    if (!result) {
        throw std::invalid_argument("binding poll requires a reconstructed keyboard/mouse device");
    }
    return *result;
}

} // namespace

void InputStateDevice::on_slot_reset() {} // 00a93e80

std::uint8_t InputStateDevice::query_1c(std::uint32_t) const {
    return 0; // 00a93ea0: XOR AL,AL; RET 4
}

void InputStateDevice::update_history_00a99e90() {
    // Interleave previous-store/query/current-store per byte, as at 00a99ea0.
    for (std::uint32_t code = 0; code != 256; ++code) {
        previous_down[code] = current_down[code];
        current_down[code] = query_20(code);
    }
}

KeyboardInputDevice::KeyboardInputDevice(::IDirectInputDevice8A* borrowed_device)
    : direct_input(borrowed_device) {}

int KeyboardInputDevice::device_class() const { return 0; }

std::uint8_t KeyboardInputDevice::query_20(std::uint32_t code) const {
    if (code >= state.size()) {
        throw std::out_of_range("native keyboard input code must be below 256");
    }
    return state[code] != 0 ? 1 : 0;
}

float KeyboardInputDevice::value_24(std::uint32_t code) const {
    return query_20(code) != 0 ? 1.0f : 0.0f;
}

bool KeyboardInputDevice::poll_00a9a4a0(const bool& suppress_keyboard) {
    state.fill(0);
    if (!direct_input) return false;
    if (FAILED(direct_input->Poll())) {
        direct_input->Acquire();
        return false;
    }
    // The original ignores this HRESULT, including partial output on failure.
    direct_input->GetDeviceState(static_cast<DWORD>(state.size()), state.data());
    for (auto& key : state) key >>= 7;
    if (suppress_keyboard) state.fill(0);
    update_history_00a99e90(); // same byte walk inlined at 00a9a540..562
    return true;
}

MouseInputDevice::MouseInputDevice(::IDirectInputDevice8A* borrowed_device,
    MouseInputGlobals& value_globals, MouseInputSample initial_sample)
    : direct_input(borrowed_device), sample(initial_sample), globals(value_globals) {
    refresh_system_settings();
}

void MouseInputDevice::refresh_system_settings() {
    swapped_buttons = GetSystemMetrics(SM_SWAPBUTTON) != 0;
    double_click_seconds = milliseconds_to_seconds(GetDoubleClickTime());
}

int MouseInputDevice::device_class() const { return 1; }

std::uint8_t MouseInputDevice::query_20(std::uint32_t code) const {
    if (!valid) return 0;
    if (code <= 7) {
        if (swapped_buttons && code <= 1) code = code == 0 ? 1 : 0;
        return sample.buttons[code] >> 7;
    }
    if (code - 8 <= 2) return value_24(code) != 0.0f ? 1 : 0;
    return 0;
}

float MouseInputDevice::value_24(std::uint32_t code) const {
    if (!valid) return 0.0f;
    if (swapped_buttons && code <= 1) code = code == 0 ? 1 : 0;
    if (code <= 7) return (sample.buttons[code] & 0x80) != 0 ? 1.0f : 0.0f;
    float axis = 0.0f;
    if (code == 8) axis = integer_mouse_axis(sample.x, false);
    else if (code == 9) {
        axis = integer_mouse_axis(sample.y, false);
        if (globals.invert_y) axis = invert_mouse_axis(axis);
    } else if (code == 10) axis = integer_mouse_axis(sample.z, true);
    // Unknown codes also execute this scaling, including 0 * NaN/Infinity.
    return scale_mouse_axis(axis, globals.axis_scale);
}

bool MouseInputDevice::poll_00a9a180(MouseInputPlatform platform) {
    valid = false;
    if (!direct_input) return false;
    if (!cooperative_configured) {
        direct_input->SetCooperativeLevel(reinterpret_cast<HWND>(platform.window),
            DISCL_EXCLUSIVE | DISCL_FOREGROUND); // native literal 5
        cooperative_configured = true; // set even on COM failure
    }
    if (platform.settings_changed) {
        platform.settings_changed = false;
        refresh_system_settings();
    }
    if (FAILED(direct_input->Poll())) {
        direct_input->Acquire();
        return false;
    }
    valid = SUCCEEDED(direct_input->GetDeviceState(sizeof(sample), &sample));
    // Even a failed GetDeviceState adds the retained/partially-written sample.
    accumulated[0] += static_cast<std::uint32_t>(sample.x);
    accumulated[1] += static_cast<std::uint32_t>(sample.y);
    accumulated[2] += static_cast<std::uint32_t>(sample.z);
    update_history_00a99e90();
    return true; // GetDeviceState failure does not change this return
}

KeyboardMouseBindingPollHost::KeyboardMouseBindingPollHost(CrtSqrt crt_sqrt)
    : sqrt_(crt_sqrt) {
    if (!sqrt_) throw std::invalid_argument("a genuine CRT sqrt call is required");
}

std::uint8_t KeyboardMouseBindingPollHost::device_query_1c(
    InputDevice& device, std::uint32_t code) {
    return query_device(device).query_1c(code);
}

std::uint8_t KeyboardMouseBindingPollHost::device_query_20(
    InputDevice& device, std::uint32_t code) {
    return query_device(device).query_20(code);
}

float KeyboardMouseBindingPollHost::device_value_24(
    InputDevice& device, std::uint32_t code) {
    return query_device(device).value_24(code);
}

float KeyboardMouseBindingPollHost::crt_sqrt_00bf7030(float value) {
    return sqrt_(value);
}

} // namespace bsp
