#pragma once

#include "bsp/input_focus_reset.hpp"
#include "bsp/native_string.hpp"

#include <Xinput.h>
#include <memory>
#include <string>

namespace bsp {

using XInputGetStateFunction = DWORD (WINAPI*)(DWORD, XINPUT_STATE*);

class XInputApi {
public:
    virtual ~XInputApi() = default;
    virtual DWORD get_state(DWORD user, XINPUT_STATE&) = 0;
    virtual DWORD set_state(DWORD user, XINPUT_VIBRATION&) = 0;
};

// Actual caller-selected XINPUT1_3 DLL; imported ordinals2/3, stdcall two args.
// No controller emulation. Keep the library alive through devices and borrowed
// get_state_function() use (also compatible with the joystick's Xbox branch).
class XInputLibrary final : public XInputApi {
public:
    explicit XInputLibrary(const std::wstring& absolute_path);
    ~XInputLibrary() override;
    DWORD get_state(DWORD, XINPUT_STATE&) override;
    DWORD set_state(DWORD, XINPUT_VIBRATION&) override;
    XInputGetStateFunction get_state_function() const noexcept;
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

// Mutable image tables are read at each native query. Defaults match the EXE.
// Supply the actual rumble flag E12F2C and CRT runtime mode0109EEA4 as bindings.
struct XInputDeviceGlobals {
    const bool& rumble_enabled;
    const bool& crt_sse2_conversion;
    std::array<std::uint16_t, 16> button_masks{{
        4096,8192,16384,32768,4,8,1,2,64,128,256,512,65535,65535,16,32}};
    std::array<const char*, 16> button_names{{
        "^FE.opt_button_a", "^FE.opt_button_b", "^FE.opt_button_x", "^FE.opt_button_y",
        "^FE.opt_button_dpad_left", "^FE.opt_button_dpad_right",
        "^FE.opt_button_dpad_up", "^FE.opt_button_dpad_down",
        "^FE.opt_button_left_stick", "^FE.opt_button_right_stick",
        "^FE.opt_button_left_bumper", "^FE.opt_button_right_bumper",
        "^FE.opt_button_left_trigger", "^FE.opt_button_right_trigger",
        "^FE.opt_button_start", "^FE.opt_button_back"}};
    std::array<const char*, 4> axis_names{{
        "^FE.opt_button_left_stick_x", "^FE.opt_button_left_stick_y",
        "^FE.opt_button_right_stick_x", "^FE.opt_button_right_stick_y"}};
};

// New typed interface. Native allocation240h, vtableD5BB48. The common base's
// force-request tree is constructed empty and remains outside this API; native
// A95A80 teardown of that empty tree has no device calls. This does not model
// arbitrary externally populated A954C0 force requests or native object layout.
class XInputDevice final : public InputStateDevice {
public:
    // A9A5A0, ECX=this, index stack, RET4. Canonical history arrays are zeroed by
    // InputStateDevice; native also zeroes sample/vibration and connection flag.
    XInputDevice(std::uint32_t user, XInputApi&, XInputDeviceGlobals&) noexcept;
    ~XInputDevice() override = default;
    int device_class() const override; // common A95BD0 ->2
    std::uint8_t query_20(std::uint32_t) const override; // A9A610, RET4
    float value_24(std::uint32_t) const override; // A9A660, RET4
    bool poll_00a9a7f0(float ignored_seconds); // thiscall, RET4, AL bool
    DWORD stop_vibration_00a9a790(); // thiscall, RET, EAX SDK result
    void set_motor_value_00a9a9c0(std::uint32_t motor, float value); // RET8
    NativeString& control_name_00a9aa40(NativeString&, std::uint32_t code,
        NativeStringStorage& = crt_string_storage()) const; // native RET8
    bool activity_00a93f30() const;
    std::int32_t identifier_00a9a5f0() const noexcept; // xorEAX/RET, always0

    std::uint32_t user_index; // +220; +224 is not accessed by these bodies
    XINPUT_STATE state{}; // +228, full16 bytes, SDK writes in place
    XINPUT_VIBRATION vibration{}; // +238, motor0 writes RIGHT, motor1 LEFT
    bool connected{}; // +23C means result != ERROR_DEVICE_NOT_CONNECTED
private:
    XInputApi& api_;
    XInputDeviceGlobals& globals_;
};

// A9A7C0: base teardown then free iff bit0; no implicit vibration reset.
// Standard-new-compatible typed storage, with the empty force-tree boundary.
XInputDevice* delete_xinput_device_00a9a7c0(XInputDevice&, std::uint32_t flags);

// Chain before JoystickFocusDeviceHost and behind KeyboardMouseFocusDeviceHost
// to provide actual poll/query/delete behavior for both class2 implementations.
class XInputFocusDeviceHost final : public InputFocusDeviceHost {
public:
    explicit XInputFocusDeviceHost(InputFocusDeviceHost* other_devices = nullptr) noexcept;
    std::int32_t query_identifier_vslot34(InputDevice&) override;
    void delete_device_vslot04(InputDevice&, std::uint32_t flags) override;
    void poll_device_vslot10(InputDevice&, float seconds) override;
    bool activity_vslot28(InputDevice&) override;
private:
    InputFocusDeviceHost& other();
    InputFocusDeviceHost* other_;
};

} // namespace bsp
