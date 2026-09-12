#pragma once

#include "bsp/xinput_device.hpp"
#include <cstddef>
#include <cstdint>

namespace bsp {

// These are the actual allocations/prefix, without implicit initialization.
// Identity DWORDs are original profiles, not source C++ vtable pointers.
struct alignas(4) NativeGamepadStorage { std::byte bytes[0x220]; };
struct alignas(4) NativeXInputStorage { std::byte bytes[0x240]; };
static_assert(sizeof(void*) == 4);
static_assert(sizeof(NativeGamepadStorage) == 0x220);
static_assert(sizeof(NativeXInputStorage) == 0x240);

// Required dispatch against the CURRENT profile of each actual allocation.
// Request receivers are raw force-request objects, never GamepadForceRequest
// C++ projections. No default implementation or empty-container assumption.
class NativeGamepadDispatch {
public:
    virtual ~NativeGamepadDispatch() = default;
    virtual void request_delete_vslot00(void* actual_request, std::uint32_t flags) = 0;
    virtual std::uint32_t request_channel_vslot04(void* actual_request) = 0;
    virtual float request_value_vslot08(void* actual_request) = 0;
    virtual bool request_expired_vslot0c(void* actual_request) = 0;
    virtual void request_update_vslot10(void* actual_request, float seconds) = 0;
    // Called only after reloading the raw device profile. D5B670's purecall
    // is handled inside this module using the real CRT service.
    virtual void set_force_vslot38(void* actual_device, std::uint32_t channel, float value) = 0;
    virtual float device_value_vslot24(void* actual_device, std::uint32_t code) = 0;
};

struct NativeGamepadContext {
    const bool& rumble_enabled_e12f2c;
    NativeGamepadDispatch& dispatch;
};
struct NativeXInputContext {
    NativeGamepadContext& gamepad;
    XInputApi& api; // production uses the existing caller-path XInputLibrary
    const XInputDeviceGlobals& tables; // borrowed live masks/name tables
    const volatile std::uint32_t& crt_sse2_conversion_0109eea4;
};

// A95D70: ECX actual220h (or larger derived prefix), EAX=this, RET.
// Writes references+4=1, histories+C/+10C, tree head+210/count+214,
// amplitudes+218/+21C. Assigned slot+8 and allocator+20C stay untouched.
void* construct_native_gamepad_00a95d70(void*, NativeGamepadContext&);
// A95A80: stamps D5B670 BEFORE payload deletion and zero-second pump.
// Full nonempty raw18h tree cleanup; no typed map or shadow ownership.
void destroy_native_gamepad_00a95a80(void*, NativeGamepadContext&);
void* scalar_delete_native_gamepad_00a95e40(void*, std::uint32_t flags, NativeGamepadContext&);
void pump_native_gamepad_force_00a954c0(void*, float seconds, NativeGamepadContext&);
void clear_native_gamepad_force_00a95890(void*, NativeGamepadContext&);
std::int32_t native_gamepad_class_00a95bd0(const void*) noexcept;

// Actual240h producer. +224 and padding+23D..23F remain untouched.
void* construct_native_xinput_00a9a5a0(void*, std::int32_t index, NativeGamepadContext&);
void destroy_native_xinput_00a9a600(void*, NativeGamepadContext&);
void* scalar_delete_native_xinput_00a9a7c0(void*, std::uint32_t flags, NativeGamepadContext&);
std::int32_t native_xinput_identifier_00a9a5f0(const void*) noexcept;
// Native RET4, result EAX0/1; vslot24 remains current-profile dispatch.
std::uint32_t query_native_xinput_00a9a610(void*, std::uint32_t code, NativeGamepadContext&);
float value_native_xinput_00a9a660(const void*, std::uint32_t code, const XInputDeviceGlobals&);
// Native RET4/AL boolean. SDK writes DIRECTLY into actual+228/+238.
bool poll_native_xinput_00a9a7f0(void*, float seconds, NativeXInputContext&);
std::uint32_t stop_native_xinput_vibration_00a9a790(void*, XInputApi&);
void set_native_xinput_motor_00a9a9c0(void*, std::uint32_t channel, float value);
NativeString& name_native_xinput_control_00a9aa40(const void*, NativeString& actual_result,
    std::uint32_t code, const XInputDeviceGlobals&, NativeStringStorage&);

// Source context/dispatch changes the callable ABI. Native reference release,
// raw profile admission, request producers, backend attachment and history
// update callers remain external; these services create no singleton domain.
// Scalar bit0 storage must belong to singleton_lifetime_allocate/free.
// C++ unwind follows the recovered member-state map; original FH3/SEH and
// arbitrary hardware-fault compatibility are not claimed.
} // namespace bsp
