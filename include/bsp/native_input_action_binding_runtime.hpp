#pragma once
#include "bsp/system_camera_axes.hpp"
#include <cstdint>

namespace bsp {
// Finite production dispatch is supplied by NativeInputDeviceRuntime. These
// calls receive the actual receiver and the profile captured at the native
// call site; a provider must not select a replacement profile after capture.
struct NativeInputActionBindingCalls {
    virtual ~NativeInputActionBindingCalls() = default;
    virtual std::uint8_t relative_vslot1c(void*, std::uint32_t profile, std::uint32_t code) = 0;
    virtual std::uint8_t query_vslot20(void*, std::uint32_t profile, std::uint32_t code) = 0;
    virtual float value_vslot24(void*, std::uint32_t profile, std::uint32_t code) = 0;
};
struct NativeInputActionBindingConstants {
    const volatile float& half_00ce3800;
    const volatile float& negative_zero_00d7a208;
    const volatile float& one_00d7a24c;
    const volatile double& quarter_00d7a348;
    const volatile double& half_00d7a280;
    const volatile double& seven_eighths_00ce42e0;
    const volatile double& eighth_00d04380;
};
struct NativeInputActionBindingContext {
    NativeInputActionBindingCalls& devices;
    const CameraAxesCrtAccess& crt;
    NativeInputActionBindingConstants constants;
};

// Actual24h owner: action header at+4, count+8, stride30h. Each action has
// binding header+10/count+14, stride34h; modifier arrays+18/+24, stride14h.
// No projected records/vectors, native allocations or additional references.
// All four entries: ECX actual receiver, no native stack arguments, RET.
// Rebind reloads F8BBF4 for each primary/modifier resolution. Only primary
// classFFFFFFFF skips lookup. Unknown classes remain native valid-range
// preconditions. Real returning CRT validation and current endpoints survive.
void rebind_native_input_action_00a91e80(void* actual_action, void* volatile& backend_00f8bbf4);
void rebind_all_native_input_actions_00a922a0(void* actual_owner, void* volatile& backend_00f8bbf4);
// Returns canonical AL0/1; examines cached primaries even on unresolved rows.
std::uint8_t query_native_input_action_relative_00a92090(void* actual_action,
    NativeInputActionBindingCalls&);
void poll_native_input_action_00a92370(void* actual_action, NativeInputActionBindingContext&);

// A91D60: native stack primary/paired float, RET8, ST0 float result; ECX/EDX
// carry no input. Preserves native binary32 spills around the shared recovered
// BF7030 CRT entry, using the caller's actual CRT binding and FPU environment.
float adjust_native_input_binding_pair_00a91d60(float primary, float paired,
    const CameraAxesCrtAccess&);

// New explicit-service C++ ABI. Caller supplies valid raw records/ranges and
// services for their entire use, including callbacks that mutate fields.
// Native bounds, exact byte predicates and reload order are preserved; no
// hardware-fault/SEH, original vtable ABI or game-runtime compatibility claim.
} // namespace bsp
