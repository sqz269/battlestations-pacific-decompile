#pragma once

#include "bsp/frame_clock.hpp"
#include "bsp/native_input_device_sdk.hpp"
#include "bsp/native_string.hpp"
#include <Xinput.h>
#include <cstddef>
#include <cstdint>

namespace bsp {

// Sole native storage, including the actual 220h gamepad prefix. No C++ vtable,
// implicit initialization, destructor, copied sample vectors or owner handles.
struct alignas(4) NativeJoystickStorage { std::byte bytes[0xb48]; };

// Borrow the original live values; no synthetic native globals are installed.
struct NativeJoystickConstants {
    const volatile float& one_00d7a24c;
    const volatile double& one_00d7a210;
    const volatile float& trigger_threshold_00ce3800;
    const volatile float& activity_threshold_00ce3868;
    const volatile double& deadzone_00ce3d10;
    const volatile double& deadzone_gain_00d5b7e8;
    const volatile double& activity_seconds_00ce3d68;
    const volatile double& trigger_maximum_00ce4b48;
    const volatile double& force_scale_00ce4bd8;
    const volatile std::uint32_t& sse2_conversion_0109eea4;
};

class NativeJoystickCalls {
public:
    virtual ~NativeJoystickCalls() = default;
    virtual void* call_00a95d70(void* actual_device) = 0;
    virtual void call_00a95a80(void* actual_device) = 0;
    virtual float call_device_vslot24(void* actual_device,
        std::uint32_t current_profile, std::uint32_t code) = 0;
    virtual const ClockTimestamp& call_clock_01090ab0_vslot14() = 0;
    virtual HWND call_00bec230() = 0;
};
using NativeJoystickXInputGetState = DWORD (WINAPI*)(DWORD, XINPUT_STATE*);
struct NativeJoystickContext {
    NativeStringStorage& strings;
    NativeInputDeviceSdk& sdk;
    NativeJoystickCalls& calls;
    NativeJoystickConstants constants;
    NativeJoystickXInputGetState xinput_get_state;
    // Explicit native stack preimage, copied only when the Xbox branch runs.
    // The real SDK writes the local copy; HRESULT is ignored as in A9910E.
    // Distinct from the caller's untouched object B40/B44 allocation bytes.
    const XINPUT_STATE& xinput_stack_preimage;
};

// Full normal bodies over valid original storage. Host C++ interfaces are not
// binary replacements for thiscall/stdcall, FH3/SEH or hardware faults.
// Constructor ECX actualB48h, stack DI8*, instance*, EAX=this, RET8. Caller owns
// allocation/free on failure. Member-array allocations remain live on later
// constructor failure, matching native EH (only name/base unwind).
void* construct_native_joystick_00a99940(void*, IDirectInput8A&,
    const DIDEVICEINSTANCEA&, NativeJoystickContext&);
void destroy_native_joystick_00a991f0(void*, NativeJoystickContext&); // ECX, RET
void* scalar_delete_native_joystick_00a99900(void*, std::uint32_t flags,
    NativeJoystickContext&); // destroy, flags&1 free original, EAX=this, RET4
void reset_native_joystick_feedback_00a98400(void*); // ECX, RET
// ECX, EAX product data or original live F8BC03 empty-byte address, RET.
const char* native_joystick_product_name_00a992e0(const void*, const char* empty_00f8bc03) noexcept;
bool poll_native_joystick_00a98e30(void*, float ignored_seconds,
    NativeJoystickContext&); // ECX, stackfloat ignored, AL, RET4
void set_native_joystick_force_00a98cc0(void*, std::uint32_t channel, float,
    NativeJoystickContext&); // ECX, channel/float, RET8

void* construct_native_joystick_object_00a99150(void*) noexcept; // only8h zero; EAX=this
void destroy_native_joystick_object_00a99160(void*, NativeStringStorage&) noexcept;
int WINAPI count_native_joystick_objects_00a98b90(const DIDEVICEOBJECTINSTANCEA*,
    void* actual_device) noexcept; // stdcall, RET8, native context identity
int describe_native_joystick_object_00a992f0(void*, const DIDEVICEOBJECTINSTANCEA&,
    NativeStringStorage&); // thiscall, EAX1, RET4
// Native callback has only raw receiver context. An active constructor-local
// TLS frame supplies borrowed strings/exception transport, never stored in it.
int WINAPI native_joystick_object_callback_00a99920(
    const DIDEVICEOBJECTINSTANCEA*, void* actual_device) noexcept;
void set_native_joystick_relative_00a98bb0(void*, std::uint32_t code,
    std::uint8_t raw_byte) noexcept; // ECX, code/byte, RET8
std::uint8_t query_native_joystick_relative_00a98750(const void*, std::uint32_t) noexcept;
std::uint8_t query_native_joystick_binding_down_00a98780(const void*, const void*) noexcept;
float query_native_joystick_binding_value_00a98940(const void*, const void*,
    const NativeJoystickConstants&) noexcept;
std::uint8_t query_native_joystick_down_00a98bd0(const void*, std::uint32_t,
    const NativeJoystickConstants&) noexcept;
float query_native_joystick_value_00a98c50(const void*, std::uint32_t,
    const NativeJoystickConstants&) noexcept;
// Queries take ECX receiver plus one DWORD stack argument and RET4; value uses
// ST0, down/relative use AL. No new code/range/pointer guards are inserted.
} // namespace bsp
