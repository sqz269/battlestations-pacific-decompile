#pragma once

#include "bsp/frame_clock.hpp"
#include "bsp/input_enumeration.hpp"

#ifndef DIRECTINPUT_VERSION
#define DIRECTINPUT_VERSION 0x0800
#endif
#include <dinput.h>
#include <Xinput.h>

#include <functional>
#include <optional>

namespace bsp {

// Imported XINPUT1_3.dll ordinal2, thunk C2F166/IAT CE239C. An actual library
// function pointer is required only by the Xbox360 product-name branch.
using JoystickXInputGetState = DWORD (__stdcall*)(DWORD, XINPUT_STATE*);
struct JoystickInputServices {
    NativeStringStorage& strings;
    InputEnumerationWindowHost& window;
    std::function<ClockTimestamp()> current_clock_01090ab0_vslot14;
    JoystickXInputGetState xinput_get_state;
    // Explicit initial storage for native uninitialized current/previous DWORDs.
    std::int32_t initial_object_value;
    // Native B40/B44 are read by the first poll's value scan before XInput runs.
    std::array<float, 2> initial_trigger_values;
};

struct JoystickAxisRange {
    std::int32_t minimum;
    std::int32_t maximum;
    std::int32_t span;
};
struct JoystickObject {
    NativeString name; // native 1Ch record +0
    std::optional<std::int32_t> kind; // +8: button0, axis1, POV2
    std::optional<JoystickAxisRange> range; // flag+C, min+10/max+14/span+18
};
struct JoystickBinding {
    std::uint32_t kind{}; // native 0Ch record: 0 absent,1 button,2 axis,3/4 halves,5..8 POV
    std::int32_t object{-1};
    bool relative{};
};

// Typed projection, not native B48h object/vtable/refcount layout. DirectInput
// device/effect COM references are BORROWED: native destruction unloads effects
// but does not Release either effects or device. The application tracks them.
class JoystickInputDevice final : public InputStateDevice {
public:
    explicit JoystickInputDevice(JoystickInputServices&);
    ~JoystickInputDevice() override;
    int device_class() const override; // A95BD0 =>2
    void on_slot_reset() override; // A98400: SendForceFeedbackCommand(2,20h,1)
    std::uint8_t query_1c(std::uint32_t code) const override; // A98750
    std::uint8_t query_20(std::uint32_t code) const override; // A98BD0
    float value_24(std::uint32_t code) const override; // A98C50
    bool poll_00a98e30(float seconds);
    bool activity_00a93f30() const;
    void set_relative_binding_00a98bb0(std::uint32_t code, bool relative);

    // A98B90/A992F0 callback state, also used by the native dynamic data format.
    IDirectInputDevice8A* direct_input{}; // +220
    NativeString product_name; // +224
    std::uint32_t object_count{}; // +22C
    std::uint32_t described_count{}; // +230, includes buttons/POVs as well as axes
    std::array<std::uint8_t, 60> used_button_slots{}; // +234
    std::int32_t next_axis_binding{1}; // +270: 1,0,3,2,4,5,...
    DIDATAFORMAT data_format{}; // +274
    bool valid{}; // +28C
    std::vector<JoystickObject> objects; // +290
    std::vector<std::int32_t> previous_state; // +294
    std::vector<std::int32_t> current_state; // +298
    std::vector<DIOBJECTDATAFORMAT> format_objects; // data_format.rgodf/+288
    std::array<JoystickBinding, 90> bindings; // +29C
    std::uint32_t effect_kind{}; // +B10: constructor0 constant,1 ramp
    std::uint32_t feedback_axis_count{}; // +B18
    std::array<DWORD, 2> feedback_axis_offsets{{0xffffffffu, 0xffffffffu}}; // +B1C
    std::array<IDirectInputEffect*, 2> effects{}; // +B24, Unload only
    bool inhibited{}; // +B2C
    std::int32_t last_input_object{}; // +B30; native initializes0, external detection can set-1
    std::optional<std::int32_t> last_input_direction; // +B34, native uninitialized
    float activity_deadline{}; // +B38: rounded clock seconds +60
    bool xbox_360{}; // +B3C
    std::optional<float> left_trigger, right_trigger; // +B40/+B44
    std::exception_ptr callback_failure; // host-only transport across SDK callback

private:
    JoystickInputServices& services_;
    std::uint8_t binding_down_00a98780(const JoystickBinding&) const;
    float binding_value_00a98940(const JoystickBinding&) const;
    friend JoystickInputDevice* create_joystick_input_device_00a99940(
        IDirectInput8A&, const DIDEVICEINSTANCEA&, JoystickInputServices&);
    friend int describe_joystick_object_00a992f0(
        JoystickInputDevice&, const DIDEVICEOBJECTINSTANCEA&);
};

int __stdcall count_joystick_objects_00a98b90(const DIDEVICEOBJECTINSTANCEA*, void*) noexcept;
int describe_joystick_object_00a992f0(JoystickInputDevice&, const DIDEVICEOBJECTINSTANCEA&);
int __stdcall joystick_object_callback_00a99920(const DIDEVICEOBJECTINSTANCEA*, void*) noexcept;
JoystickInputDevice* create_joystick_input_device_00a99940(
    IDirectInput8A&, const DIDEVICEINSTANCEA&, JoystickInputServices&);
void delete_joystick_input_device_00a99900(JoystickInputDevice&, std::uint32_t flags);

// Delegate for the canonical KeyboardMouseFocusDeviceHost's unsupported-device
// boundary. Uses actual joystick polls/queries/deletion; its +34 identity is the
// already reconstructed zero-return helper A93EB0.
class JoystickFocusDeviceHost final : public InputFocusDeviceHost {
public:
    std::int32_t query_identifier_vslot34(InputDevice&) override;
    void delete_device_vslot04(InputDevice&, std::uint32_t flags) override;
    void poll_device_vslot10(InputDevice&, float seconds) override;
    bool activity_vslot28(InputDevice&) override;
};

} // namespace bsp
