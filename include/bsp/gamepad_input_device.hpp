#pragma once

#include "bsp/gamepad_force_requests.hpp"
#include "bsp/input_device_state.hpp"

namespace bsp {

// Canonical common gamepad state shared by actual joystick and XInput classes.
// Native A95D70 constructs an empty request tree and zero amplitudes. Requests
// enter this object through GamepadForceDeviceHost, which binds the same E12F2C
// flag that the application uses for force updates. No shadow registry.
class GamepadInputDevice : public InputStateDevice {
public:
    ~GamepadInputDevice() override;
    GamepadForceState force;
    virtual void write_gamepad_force(std::uint32_t channel, float value) = 0;
    void bind_force_enabled(const bool&);
protected:
    explicit GamepadInputDevice(const bool* force_enabled = nullptr) noexcept;
private:
    const bool* force_enabled_;
};

class GamepadForceDeviceHost final : public GamepadForceHost {
public:
    using CurrentGroups = std::function<InputBindingDeviceGroups*()>;
    GamepadForceDeviceHost(bool& enabled_e12f2c, CurrentGroups);
    InputBindingDeviceGroups* current_input_groups_00f8bbf4() override;
    GamepadForceState& force_state(InputDevice&) override;
    void set_force_vslot38(InputDevice&, std::uint32_t channel, float value) override;
    GamepadForceContext context(std::uint32_t& next_id_e12f30) noexcept;
private:
    GamepadInputDevice& bind(InputDevice&);
    bool& enabled_;
    CurrentGroups current_groups_;
};

} // namespace bsp
