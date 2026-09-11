#pragma once

#include "bsp/input_focus_reset.hpp"
#include "bsp/native_string.hpp"

#include <array>
#include <cstddef>
#include <exception>
#include <vector>

struct DIDEVICEINSTANCEA;
struct IDirectInput8A;

namespace bsp {

using InputInstanceGuid = std::array<std::byte, 16>;

// Production implementations call the concrete keyboard/mouse constructors
// below. Joystick 00A99940 requires its genuine device implementation; returning
// a fabricated InputDevice is not an implementation of that boundary.
class InputEnumerationDeviceFactory {
public:
    virtual ~InputEnumerationDeviceFactory() = default;
    virtual KeyboardInputDevice* create_keyboard_00a9a3e0() = 0;
    virtual MouseInputDevice* create_mouse_00a9a290() = 0;
    virtual InputDevice* create_joystick_00a99940(const ::DIDEVICEINSTANCEA&) = 0;
};

struct InputEnumerationContext {
    InputFocusBackendState& backend;
    // Actual existing manager GUID sequence (+E8 begin/+EC end), retained across
    // re-enumeration. This is not the active-device vector or a fresh local set.
    std::vector<InputInstanceGuid>& seen_instance_guids;
    NativeStringStorage& strings;
    InputEnumerationDeviceFactory& devices;
    // Host-only transport: exceptions cannot unwind through a DirectInput DLL.
    std::exception_ptr callback_failure;
};

// A972E0, ECX/EDX point to 16 bytes, EAX boolean, RET. Ordinary byte equality.
bool input_instance_guids_equal_00a972e0(const InputInstanceGuid&,
    const InputInstanceGuid&) noexcept;

// A98030 is stdcall(context, instance), RET8; always returns 1 on native normal
// paths. Uses the canonical owning table and records new GUIDs before filtering.
// Invalid native pointers, allocation failures and a GUID index >=8 are explicit
// host errors instead of native null dereferences or out-of-row table accesses.
int on_input_device_enumerated_00a98030(InputEnumerationContext&,
    const ::DIDEVICEINSTANCEA&);

// A982B0 is stdcall(instance, context), RET8. Normal result is DIENUM_CONTINUE.
// A host error is captured in callback_failure and returns DIENUM_STOP; the
// adapter below rethrows it after the actual EnumDevices call has returned.
int __stdcall input_enum_devices_callback_00a982b0(
    const ::DIDEVICEINSTANCEA*, void* context) noexcept;

// Concrete bridge for DirectInputHost::enum_devices. It calls the real ANSI
// COM interface with the callback and THIS context, resets callback_failure,
// and rethrows captured host errors. It does not alter backend +F4 itself.
std::int32_t enumerate_direct_input_devices(InputEnumerationContext&,
    ::IDirectInput8A&, std::uint32_t device_type, std::uint32_t flags);

// Read current platform HWND at the native point after SetDataFormat, rather
// than capturing a possibly replaced platform singleton before CreateDevice.
class InputEnumerationWindowHost {
public:
    virtual ~InputEnumerationWindowHost() = default;
    virtual HWND current_platform_window_00bec230() = 0;
};

// Allocate standard-new-compatible typed storage, invoke real CreateDevice and
// SetDataFormat with SDK GUID_SysKeyboard/c_dfDIKeyboard or SysMouse/DIMouse2.
// Keyboard then sets cooperation(HWND,6); mouse sets none here. HRESULTs are
// ignored as native, but a missing CreateDevice output is an explicit error.
//
// The returned wrapper BORROWS its direct_input pointer. The successful
// CreateDevice call supplies one COM reference: the application must retain
// that reference through wrapper use and track its release separately, including
// after a focus reset destroys the wrapper. No AddRef/Release is inserted here.
// Native mouse sample bytes are uninitialized; its initial sample is explicitly
// supplied by the typed caller. Native vtable/refcount layout is not reproduced.
KeyboardInputDevice* create_keyboard_input_device_00a9a3e0(
    ::IDirectInput8A&, InputEnumerationWindowHost&);
MouseInputDevice* create_mouse_input_device_00a9a290(
    ::IDirectInput8A&, MouseInputGlobals&, MouseInputSample initial_sample);

} // namespace bsp
