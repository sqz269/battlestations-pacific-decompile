#include "bsp/gamepad_input_device.hpp"

#include <exception>
#include <stdexcept>
#include <utility>

namespace bsp {

GamepadInputDevice::GamepadInputDevice(const bool* enabled) noexcept : force_enabled_(enabled) {}

GamepadInputDevice::~GamepadInputDevice() {
    if (force_enabled_ != nullptr) {
        // Runs after the complete derived destructor, where native A95A80 has
        // installed its base vtable. A purecall lifetime violation is fatal;
        // do not dispatch output into an already destroyed joystick or pad.
        try { destroy_gamepad_force_state_00a95a80(force, *force_enabled_); }
        catch (...) { std::terminate(); }
    } else if (!force.requests.empty() || force.amplitudes[0] != 0.0f || force.amplitudes[1] != 0.0f) {
        std::terminate(); // application bypassed the required canonical binding
    }
}

void GamepadInputDevice::bind_force_enabled(const bool& enabled) {
    if (force_enabled_ != nullptr && force_enabled_ != &enabled)
        throw std::logic_error("Gamepad force and device polling use different enable flags");
    force_enabled_ = &enabled;
}

GamepadForceDeviceHost::GamepadForceDeviceHost(bool& enabled, CurrentGroups current_groups)
    : enabled_(enabled), current_groups_(std::move(current_groups)) {
    if (!current_groups_) throw std::invalid_argument("Gamepad forces require current input groups");
}

GamepadInputDevice& GamepadForceDeviceHost::bind(InputDevice& device) {
    auto* pad = dynamic_cast<GamepadInputDevice*>(&device);
    if (pad == nullptr) throw std::logic_error("Force request requires an actual gamepad device");
    pad->bind_force_enabled(enabled_);
    return *pad;
}

InputBindingDeviceGroups* GamepadForceDeviceHost::current_input_groups_00f8bbf4() {
    return current_groups_();
}

GamepadForceState& GamepadForceDeviceHost::force_state(InputDevice& device) {
    return bind(device).force;
}

void GamepadForceDeviceHost::set_force_vslot38(InputDevice& device, std::uint32_t channel, float value) {
    bind(device).write_gamepad_force(channel, value);
}

GamepadForceContext GamepadForceDeviceHost::context(std::uint32_t& next_id) noexcept {
    return {enabled_, next_id, *this};
}

} // namespace bsp
