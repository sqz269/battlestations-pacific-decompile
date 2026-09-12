#include "bsp/native_input_device_runtime.hpp"
#include "bsp/native_input_guid_storage.hpp"
#include "bsp/input_focus_reset.hpp"
#include "bsp/sound_system_update.hpp"

#include <cstdlib>
#include <exception>
#include <stdexcept>

namespace bsp {
namespace {
constexpr std::uint32_t keyboard = 0x00d5b904;
constexpr std::uint32_t mouse = 0x00d5b8b0;
constexpr std::uint32_t xinput = 0x00d5bb48;
constexpr std::uint32_t joystick = 0x00d5b7f0;
constexpr std::uint32_t gamepad = 0x00d5b670;
std::uint32_t profile(const void* owner) noexcept {
    return *static_cast<const volatile std::uint32_t*>(owner);
}
bool admitted(std::uint32_t p) noexcept {
    return p == keyboard || p == mouse || p == xinput || p == joystick || p == gamepad;
}
[[noreturn]] void unbound() {
    throw std::invalid_argument("unbound raw input device profile or virtual signature");
}
[[noreturn]] void pure_slot() {
    (void)_purecall();
    std::terminate(); // CRT purecall is non-returning, including after its hook.
}
IDirectInput8A& require_interface(IDirectInput8A* input) {
    if (!input) throw std::invalid_argument("raw input constructor requires its captured DirectInput interface");
    return *input;
}
} // namespace

NativeInputDeviceRuntime::NativeInputDeviceRuntime(NativeInputDeviceRuntimeServices s)
    : NativeGamepadForceRequestDispatch(s.infinite_remaining_00d7a278), services_(s),
      gamepad_{s.xinput_tables.rumble_enabled, *this},
      xinput_{gamepad_, s.xinput, s.xinput_tables, s.joystick.sse2_conversion_0109eea4},
      keyboard_mouse_{s.platform_0109cf04, *this, s.keyboard_mouse},
      joystick_{s.strings, s.sdk, *this, s.joystick, s.xinput.get_state_function(),
          s.joystick_xinput_stack_preimage}, enumeration_{s.strings, *this} {
    if (&s.keyboard_mouse.one_00d7a24c != &s.joystick.one_00d7a24c ||
        &s.keyboard_mouse.negative_zero_00d7a208 != &s.selection.negative_zero_00d7a208)
        throw std::invalid_argument("raw input services must share each canonical image-value word");
    if (!s.names.keyboard_00d5b6fc || !s.names.mouse_00d5b8a4 ||
        !s.names.gamepad_00d5b6ac || !s.empty_00f8bc03)
        throw std::invalid_argument("raw input names require their canonical borrowed literal addresses");
}

void NativeInputDeviceRuntime::enumerate_devices_vslot10(void* input,
    std::uint32_t type, void* backend, std::uint32_t flags) {
    (void)enumerate_native_input_devices(require_interface(static_cast<IDirectInput8A*>(input)),
        type, backend, flags, enumeration_);
}
void* NativeInputDeviceRuntime::construct_xinput_device_00a9a5a0(void* device, std::int32_t index) {
    return construct_native_xinput_00a9a5a0(device, index, gamepad_);
}
void NativeInputDeviceRuntime::attach_device_00a904e0(void* backend, std::int32_t slot, void* device) {
    attach_native_input_device_00a904e0(backend, static_cast<std::uint32_t>(slot), device, *this);
}
void NativeInputDeviceRuntime::reset_device_vslot14(void* device) {
    const auto p = profile(device);
    if (p == joystick) reset_native_joystick_feedback_00a98400(device);
    else if (admitted(p)) reset_native_input_device_00a93e80(device);
    else unbound();
}
void NativeInputDeviceRuntime::zero_references_device_vslot00(void* device) {
    if (!admitted(profile(device))) unbound();
    // The backend already decremented actual+4. Slot00 BD30E0 performs its
    // own current-profile read and invokes captured slot04(flags1), once.
    invoke_native_ref_counted_delete_00bd30e0(device, *this);
}
std::uint32_t NativeInputDeviceRuntime::device_class_vslot08(void* device, std::uint32_t p) {
    if (p == keyboard) return native_keyboard_class_00a96350();
    if (p == mouse) return native_mouse_class_00a9a0f0();
    if (p == xinput || p == joystick || p == gamepad)
        return static_cast<std::uint32_t>(native_gamepad_class_00a95bd0(device));
    unbound();
}
void* NativeInputDeviceRuntime::construct_keyboard_00a9a3e0(void* device, IDirectInput8A* input) {
    return construct_native_keyboard_00a9a3e0(device, require_interface(input), services_.sdk, keyboard_mouse_);
}
void* NativeInputDeviceRuntime::construct_mouse_00a9a290(void* device, IDirectInput8A* input) {
    return construct_native_mouse_00a9a290(device, require_interface(input), services_.sdk, keyboard_mouse_);
}
void* NativeInputDeviceRuntime::construct_joystick_00a99940(void* device,
    IDirectInput8A* input, const DIDEVICEINSTANCEA& instance) {
    return construct_native_joystick_00a99940(device, require_interface(input), instance, joystick_);
}
void NativeInputDeviceRuntime::append_guid_00a97fa0(void* header, const GUID& guid) {
    append_input_guid_storage(header, guid);
}
void NativeInputDeviceRuntime::invalid_parameter_00bf6713() { _invalid_parameter_noinfo(); }

std::uint8_t NativeInputDeviceRuntime::query_20(void* d, std::uint32_t code) {
    return query_vslot20(d, profile(d), code);
}
float NativeInputDeviceRuntime::value_24(void* d, std::uint32_t code) {
    return value_vslot24(d, profile(d), code);
}
std::uint8_t NativeInputDeviceRuntime::query_vslot20(void* d, std::uint32_t p, std::uint32_t code) {
    switch (p) {
    case keyboard: return query_native_keyboard_00a95e70(d, code);
    case mouse: return query_native_mouse_00a99f70(d, code, keyboard_mouse_);
    case xinput: return static_cast<std::uint8_t>(query_native_xinput_00a9a610(d, code, gamepad_));
    case joystick: return query_native_joystick_down_00a98bd0(d, code, services_.joystick);
    case gamepad: pure_slot();
    default: unbound();
    }
}
float NativeInputDeviceRuntime::value_vslot24(void* d, std::uint32_t p, std::uint32_t code) {
    switch (p) {
    case keyboard: return value_native_keyboard_00a95e90(d, code, keyboard_mouse_);
    case mouse: return value_native_mouse_00a99fe0(d, code, keyboard_mouse_.globals);
    case xinput: return value_native_xinput_00a9a660(d, code, services_.xinput_tables);
    case joystick: return query_native_joystick_value_00a98c50(d, code, services_.joystick);
    case gamepad: pure_slot();
    default: unbound();
    }
}
std::uint8_t NativeInputDeviceRuntime::activity_vslot28(void* d, std::uint32_t p) {
    if (p == keyboard) return native_keyboard_has_activity_00a95ed0(d, *this);
    if (p == mouse) return native_mouse_has_activity_00a9aab0(d, *this);
    if (p == xinput || p == joystick || p == gamepad) return native_gamepad_has_activity_00a93f30(d, *this);
    unbound();
}
void NativeInputDeviceRuntime::delete_vslot04(void* d, std::uint32_t p, std::uint32_t flags) {
    switch (p) {
    case keyboard: (void)delete_native_keyboard_00a9a470(d, static_cast<std::uint8_t>(flags)); return;
    case mouse: (void)delete_native_mouse_00a9a390(d, static_cast<std::uint8_t>(flags)); return;
    case xinput: (void)scalar_delete_native_xinput_00a9a7c0(d, flags, gamepad_); return;
    case joystick: (void)scalar_delete_native_joystick_00a99900(d, flags, joystick_); return;
    case gamepad: (void)scalar_delete_native_gamepad_00a95e40(d, flags, gamepad_); return;
    default: unbound();
    }
}

void* NativeInputDeviceRuntime::call_00a95d70(void* d) { return construct_native_gamepad_00a95d70(d, gamepad_); }
void NativeInputDeviceRuntime::call_00a95a80(void* d) { destroy_native_gamepad_00a95a80(d, gamepad_); }
float NativeInputDeviceRuntime::call_device_vslot24(void* d, std::uint32_t p, std::uint32_t code) {
    return value_vslot24(d, p, code);
}
const ClockTimestamp& NativeInputDeviceRuntime::call_clock_01090ab0_vslot14() {
    auto* const clock = services_.clock_01090ab0;
    if (!clock) throw std::logic_error("raw input current-clock publication is null");
    return *sound_frame_clock_current_00bee050(*clock);
}
HWND NativeInputDeviceRuntime::call_00bec230() {
    auto* const platform = services_.platform_0109cf04;
    if (!platform) throw std::logic_error("raw input platform publication is null");
    return get_platform_window_00bec230(*platform);
}
void NativeInputDeviceRuntime::set_force_vslot38(void* d, std::uint32_t channel, float value) {
    switch (profile(d)) {
    case xinput: set_native_xinput_motor_00a9a9c0(d, channel, value); return;
    case joystick: set_native_joystick_force_00a98cc0(d, channel, value, joystick_); return;
    case gamepad: pure_slot();
    default: unbound();
    }
}
float NativeInputDeviceRuntime::device_value_vslot24(void* d, std::uint32_t code) {
    return value_vslot24(d, profile(d), code);
}

const char* NativeInputDeviceRuntime::device_name_vslot0c(void* d) {
    switch (profile(d)) {
    case keyboard: return native_keyboard_device_name_00a96360(d, services_.names);
    case mouse: return native_mouse_device_name_00a9a100(d, services_.names);
    case xinput: case gamepad: return native_gamepad_device_name_00a95be0(d, services_.names);
    case joystick: return native_joystick_product_name_00a992e0(d, services_.empty_00f8bc03);
    default: unbound();
    }
}
bool NativeInputDeviceRuntime::poll_device_vslot10(void* d, float seconds) {
    switch (profile(d)) {
    case keyboard: return poll_native_keyboard_00a9a4a0(d, keyboard_mouse_);
    case mouse: return poll_native_mouse_00a9a180(d, keyboard_mouse_);
    case xinput: return poll_native_xinput_00a9a7f0(d, seconds, xinput_);
    case joystick: return poll_native_joystick_00a98e30(d, seconds, joystick_);
    case gamepad: pure_slot();
    default: unbound();
    }
}
void NativeInputDeviceRuntime::set_relative_vslot18(void* d, std::uint32_t code, std::uint32_t value) {
    const auto p = profile(d);
    if (p == joystick) set_native_joystick_relative_00a98bb0(d, code, static_cast<std::uint8_t>(value));
    else if (admitted(p)) set_native_input_relative_noop_00a93e90(d, code, value);
    else unbound();
}
std::uint8_t NativeInputDeviceRuntime::relative_vslot1c(void* d, std::uint32_t code) {
    const auto p = profile(d);
    if (p == joystick) return query_native_joystick_relative_00a98750(d, code);
    if (admitted(p)) return query_native_input_relative_false_00a93ea0(d, code);
    unbound();
}
std::uint8_t NativeInputDeviceRuntime::buttons_active_vslot2c(void* d) {
    switch (profile(d)) {
    case keyboard: return native_keyboard_buttons_active_00a95f00(d, *this);
    case mouse: return native_mouse_buttons_active_00a9aae0(d, *this);
    case xinput: case joystick: case gamepad: return native_gamepad_buttons_active_00a93f60(d, *this);
    default: unbound();
    }
}
std::int32_t NativeInputDeviceRuntime::select_control_vslot30(void* d) {
    switch (profile(d)) {
    case keyboard: return select_native_keyboard_control_00a95f10(d, *this);
    case mouse: return select_native_mouse_control_00a9ab10(d, *this, services_.selection);
    case xinput: case joystick: case gamepad: return select_native_gamepad_control_00a94440(d, *this, services_.selection);
    default: unbound();
    }
}
std::int32_t NativeInputDeviceRuntime::identifier_vslot34(void* d) {
    const auto p = profile(d);
    if (p == xinput) return native_xinput_identifier_00a9a5f0(d);
    if (admitted(p)) return input_device_identifier_zero_00a93eb0();
    unbound();
}
float NativeInputDeviceRuntime::mouse_double_click_vslot38(void* d) {
    if (profile(d) != mouse) unbound();
    return native_mouse_double_click_seconds_00a9a380(d);
}
} // namespace bsp
