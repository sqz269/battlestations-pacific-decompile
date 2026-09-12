#pragma once

#include "bsp/native_gamepad_force_requests.hpp"
#include "bsp/native_input_backend_owner.hpp"
#include "bsp/native_input_device_virtuals.hpp"
#include "bsp/native_input_enumeration.hpp"
#include "bsp/native_joystick.hpp"
#include "bsp/native_keyboard_mouse.hpp"
#include "bsp/native_ref_counted.hpp"

namespace bsp {

// Stable borrowed application services. Clock/window are the existing canonical
// source owners/publications used by the rest of the reconstruction; no native
// FrameClock/Win32PlatformState ABI cast is made. Fields naming the same image
// word must borrow the same storage. DLL, SDK tracker, strings and every reference
// outlive all devices, callbacks, raw backend drain and explicit SDK release.
struct NativeInputDeviceRuntimeServices {
    NativeStringStorage& strings;
    NativeInputDeviceSdk& sdk;
    XInputLibrary& xinput;
    const XInputDeviceGlobals& xinput_tables;
    Win32PlatformState* volatile& platform_0109cf04;
    FrameClock* volatile& clock_01090ab0;
    NativeKeyboardMouseGlobals keyboard_mouse;
    NativeJoystickConstants joystick;
    NativeInputSelectionConstants selection;
    NativeInputDeviceNameLiterals names;
    const char* empty_00f8bc03;
    const volatile double& infinite_remaining_00d7a278;
    const XINPUT_STATE& joystick_xinput_stack_preimage;
};

// Source composition of recovered providers, not another native function or
// arbitrary-vtable emulator. Exactly five device profiles are admitted:
// keyboard D5B904, mouse D5B8B0, XInput D5BB48, joystick D5B7F0, common D5B670.
// No raw allocations, singleton manager, device table, GUID owner or implicit
// COM Release lives in this dispatcher. Slot coverage is explicit below; table
// tails use profile-specific signatures. Its private contexts only borrow the
// services above; native receivers and outputs remain their actual allocations.
class NativeInputDeviceRuntime final : public NativeInputBackendHost,
    public NativeInputEnumerationCalls, public NativeInputHistoryHost,
    public NativeInputDeviceVirtualCalls, public NativeJoystickCalls,
    public NativeRefCountedDeleteCalls, public NativeGamepadForceRequestDispatch {
public:
    explicit NativeInputDeviceRuntime(NativeInputDeviceRuntimeServices);
    NativeInputDeviceRuntime(const NativeInputDeviceRuntime&) = delete;
    NativeInputDeviceRuntime& operator=(const NativeInputDeviceRuntime&) = delete;

    void enumerate_devices_vslot10(void*, std::uint32_t type, void* backend,
        std::uint32_t flags) override;
    void* construct_xinput_device_00a9a5a0(void*, std::int32_t index) override;
    void attach_device_00a904e0(void*, std::int32_t slot, void*) override;
    void reset_device_vslot14(void*) override;
    void zero_references_device_vslot00(void*) override;

    std::uint32_t device_class_vslot08(void*, std::uint32_t captured_profile) override;
    void* construct_keyboard_00a9a3e0(void*, IDirectInput8A*) override;
    void* construct_mouse_00a9a290(void*, IDirectInput8A*) override;
    void* construct_joystick_00a99940(void*, IDirectInput8A*, const DIDEVICEINSTANCEA&) override;
    void append_guid_00a97fa0(void* actual_header, const _GUID&) override;
    void invalid_parameter_00bf6713() override;

    // Providers without a captured-profile argument read raw+0 on entry. A
    // provider given a captured profile selects that fixed target without
    // reading the profile again. Nested native virtual calls capture anew.
    std::uint8_t query_20(void*, std::uint32_t code) override;
    float value_24(void*, std::uint32_t code) override;
    std::uint8_t query_vslot20(void*, std::uint32_t captured_profile, std::uint32_t code) override;
    float value_vslot24(void*, std::uint32_t captured_profile, std::uint32_t code) override;
    std::uint8_t activity_vslot28(void*, std::uint32_t captured_profile) override;
    void delete_vslot04(void*, std::uint32_t captured_profile, std::uint32_t flags) override;

    void* call_00a95d70(void*) override;
    void call_00a95a80(void*) override;
    float call_device_vslot24(void*, std::uint32_t captured_profile, std::uint32_t code) override;
    const ClockTimestamp& call_clock_01090ab0_vslot14() override;
    HWND call_00bec230() override;
    void set_force_vslot38(void*, std::uint32_t channel, float value) override;
    float device_value_vslot24(void*, std::uint32_t code) override;

    // Remaining finite virtual surfaces for the raw backend/frame consumers.
    const char* device_name_vslot0c(void*);
    bool poll_device_vslot10(void*, float seconds);
    void set_relative_vslot18(void*, std::uint32_t code, std::uint32_t raw_value);
    std::uint8_t relative_vslot1c(void*, std::uint32_t code);
    std::uint8_t buttons_active_vslot2c(void*);
    std::int32_t select_control_vslot30(void*);
    std::int32_t identifier_vslot34(void*);
    // Only the mouse has this signature at38. Keyboard has no slot38; gamepad
    // families use force output there. Signature/profile mismatches are errors.
    float mouse_double_click_vslot38(void*);
    // Slot3C is a string result for XInput/joystick, a DWORD for mouse.
    NativeString& control_name_vslot3c(void*, NativeString& actual_output, std::uint32_t code);
    std::uint32_t mouse_accumulated_x_vslot3c(void*);
    std::uint32_t mouse_accumulated_y_vslot40(void*);
    std::uint32_t mouse_accumulated_z_vslot44(void*);
    void set_mouse_accumulated_x_vslot48(void*, std::uint32_t bits);
    void set_mouse_accumulated_y_vslot4c(void*, std::uint32_t bits);
    void set_mouse_accumulated_z_vslot50(void*, std::uint32_t bits);
private:
    NativeInputDeviceRuntimeServices services_;
    NativeGamepadContext gamepad_;
    NativeXInputContext xinput_;
    NativeKeyboardMouseContext keyboard_mouse_;
    NativeJoystickContext joystick_;
    NativeInputEnumerationContext enumeration_;
};
} // namespace bsp
