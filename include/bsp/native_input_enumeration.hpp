#pragma once

#include "bsp/native_string.hpp"
#include <cstdint>

struct DIDEVICEINSTANCEA;
struct IDirectInput8A;
struct _GUID;

namespace bsp {
// Required services act on the same F8h backend and raw device allocations.
// Profile DWORDs identify original targets; they are not source C++ vtables.
struct NativeInputEnumerationCalls {
    virtual ~NativeInputEnumerationCalls() = default;
    virtual std::uint32_t device_class_vslot08(void* actual_device,
        std::uint32_t captured_profile) = 0;
    virtual void* construct_keyboard_00a9a3e0(void* allocation,
        IDirectInput8A* captured_direct_input) = 0;
    virtual void* construct_mouse_00a9a290(void* allocation,
        IDirectInput8A* captured_direct_input) = 0;
    virtual void* construct_joystick_00a99940(void* allocation,
        IDirectInput8A* captured_direct_input, const DIDEVICEINSTANCEA&) = 0;
    // Recognized STL contract. Append to THIS actual10h header (+4/+8/+C),
    // preserving its leading DWORD. No separately owning std::vector is valid.
    // This packet does not supply the allocator/iterator implementation.
    virtual void append_guid_00a97fa0(void* actual_vector_header,
        const _GUID& actual_guid) = 0;
    // BF6713 can return; the caller then reloads the current vector begin.
    virtual void invalid_parameter_00bf6713() = 0;
};
struct NativeInputEnumerationContext {
    NativeStringStorage& strings;
    NativeInputEnumerationCalls& calls;
};

// Native ECX backend, stack slot/device, RET8. No retain/replacement cleanup,
// bounds clamp or null guard. Full class row resolves -1 to index8 and stores
// there. The caller must provide backing storage for the native computed slot.
void attach_native_input_device_00a904e0(void* actual_backend,
    std::uint32_t requested_slot, void* actual_device, NativeInputEnumerationCalls&);

// Native stdcall(backend,instance), RET8, normal result DIENUM_CONTINUE.
// Added context changes the source ABI. Raw byte/header access and captured
// constructor/attach sequencing; no InputDevice projection or shadow GUID owner.
int on_native_input_device_enumerated_00a98030(void* actual_backend,
    const DIDEVICEINSTANCEA&, NativeInputEnumerationContext&);

// Native A982B0 forwards actual(instance,backend), stdcall RET8. A source-only
// thread-local synchronous EnumDevices frame supplies borrowed services and
// transports C++ callback failure back across the SDK boundary. Call through
// enumerate_native_input_devices; outside its frame the callback stops.
int __stdcall native_input_enum_callback_00a982b0(
    const DIDEVICEINSTANCEA*, void* actual_backend) noexcept;
std::int32_t enumerate_native_input_devices(IDirectInput8A&, std::uint32_t type,
    void* actual_backend, std::uint32_t flags, NativeInputEnumerationContext&);
} // namespace bsp
