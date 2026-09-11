#pragma once

#include "bsp/frame_clock.hpp"
#include "bsp/input_action_classifier.hpp"

#ifndef DIRECTINPUT_VERSION
#define DIRECTINPUT_VERSION 0x0800
#endif
#include <dinput.h>

#include <array>
#include <functional>
#include <map>
#include <memory>

namespace bsp {

// Native request vslots: deleting dtor0, channel4, value8, expiredC, update10.
// Typed API, not a native request/vtable layout. Three actual game classes below.
class GamepadForceRequest {
public:
    virtual ~GamepadForceRequest() = default;
    virtual std::uint32_t channel() const = 0;
    virtual float value() const = 0;
    virtual bool expired() const = 0;
    virtual void update(float seconds) = 0;
    // A94BC0 writes the full DWORD+C regardless of concrete request kind.
    std::uint32_t payload_0c{};
};

class ConstantGamepadForceRequest final : public GamepadForceRequest {
public:
    ConstantGamepadForceRequest(std::uint32_t channel, float amplitude, float remaining);
    std::uint32_t channel() const override;
    float value() const override;
    bool expired() const override;
    void update(float seconds) override;
    std::uint32_t channel_08;
    float remaining_10;
};
class FadingGamepadForceRequest final : public GamepadForceRequest {
public:
    FadingGamepadForceRequest(std::uint32_t channel, float amplitude, float duration);
    std::uint32_t channel() const override;
    float value() const override;
    bool expired() const override;
    void update(float seconds) override;
    std::uint32_t channel_08;
    float duration_10;
    float elapsed_14{};
};
class AlternatingGamepadForceRequest final : public GamepadForceRequest {
public:
    AlternatingGamepadForceRequest(std::uint32_t channel, bool second_first,
        float first_value, float second_value, float first_period,
        float second_period, float duration);
    std::uint32_t channel() const override;
    float value() const override;
    bool expired() const override;
    void update(float seconds) override;
    std::uint32_t channel_08;
    float first_value_10, second_value_14;
    float first_period_18, second_period_1c, duration_20;
    float phase_24{};
};

// Exactly one instance per actual joystick/XInput device. The application calls
// destroy_gamepad_force_state_00a95a80 at the actual base-destruction boundary.
// Enabled nonzero amplitudes there hit the native base vtable's purecall slot;
// the typed function reports that invalid state. Explicit A95890 clear still
// runs against the live derived output and can establish zero amplitudes first.
// The C++ destructor also destroys residual requests in ascending key order;
// it cannot issue device output because device lifetime is externally bound.
struct GamepadForceState {
    std::map<std::uint32_t, std::unique_ptr<GamepadForceRequest>> requests;
    std::array<float, 2> amplitudes{}; // actual native218/21C state, no shadow copy
    ~GamepadForceState();
};
class GamepadForceHost {
public:
    virtual ~GamepadForceHost() = default;
    virtual InputBindingDeviceGroups* current_input_groups_00f8bbf4() = 0;
    virtual GamepadForceState& force_state(InputDevice&) = 0;
    virtual void set_force_vslot38(InputDevice&, std::uint32_t channel, float value) = 0;
};
struct GamepadForceContext {
    bool& enabled_e12f2c;
    std::uint32_t& next_id_e12f30;
    GamepadForceHost& host;
};

void refresh_gamepad_force_channel_00a949a0(InputDevice&, std::uint32_t channel,
    GamepadForceContext&);
GamepadForceRequest* find_gamepad_force_request_00a94b50(GamepadForceState&,
    std::uint32_t id) noexcept;
bool set_gamepad_force_payload_00a94bc0(InputDevice&, std::uint32_t id,
    float value, GamepadForceContext&);
void set_gamepad_force_enabled_00a94c50(bool, GamepadForceContext&);
GamepadForceRequest* find_current_gamepad_force_request_00a94d40(
    std::uint32_t id, GamepadForceContext&);
bool remove_gamepad_force_request_00a95410(InputDevice&, std::uint32_t id,
    GamepadForceContext&);
void pump_gamepad_force_requests_00a954c0(InputDevice&, float seconds,
    GamepadForceContext&);
// Native insert ignores duplicate-key failure and leaves the incoming request
// unowned in that case. Return value makes that ownership result reviewable;
// it is a typed extension, not a recovered native return contract.
bool insert_gamepad_force_request_00a95780(InputDevice&, std::uint32_t id,
    GamepadForceRequest*, GamepadForceContext&);
void validate_gamepad_force_handle_00a957d0(std::uint32_t&, GamepadForceContext&);
void remove_current_gamepad_force_request_00a957f0(std::uint32_t id,
    GamepadForceContext&);
void clear_gamepad_force_requests_00a95890(InputDevice&, GamepadForceContext&);
void pump_current_gamepad_force_requests_00a95960(float seconds, GamepadForceContext&);
void destroy_gamepad_force_state_00a95a80(InputDevice&, GamepadForceContext&);
// Consumes the request on absent device (deletes it) or successful insertion.
// Native duplicate keys leave it unowned; callers must prevent exhausted-ID
// collisions or independently retain that incoming allocation.
std::uint32_t submit_gamepad_force_request_00a95bf0(std::uint32_t device_index,
    GamepadForceRequest*, GamepadForceContext&);

struct JoystickForceOutputState {
    std::array<IDirectInputEffect*, 2>& effects_b24;
    std::uint32_t& effect_kind_b10;
    std::int32_t& direction_mode_b14; // native constructor initializes zero at A99998
    float& activity_deadline_b38;
    std::function<ClockTimestamp()> current_clock_01090ab0_vslot14;
};
// Actual DirectInput effect SetParameters, not a production callback substitute.
// A99710/vslot3C is a separate binding-label getter, not a force command.
void set_joystick_force_00a98cc0(JoystickForceOutputState&, std::uint32_t channel,
    float value);

} // namespace bsp
