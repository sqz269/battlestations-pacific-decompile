#pragma once

#include "bsp/native_input_device_sdk.hpp"
#include "bsp/platform_window.hpp"

#include <cstddef>
#include <cstdint>

namespace bsp {

inline constexpr std::size_t native_keyboard_size = 0x310;
inline constexpr std::size_t native_mouse_size = 0x23c;
inline constexpr std::uint32_t native_keyboard_profile = 0x00d5b904;
inline constexpr std::uint32_t native_mouse_profile = 0x00d5b8b0;

// Calls through the current raw object's vtable, NOT a cached device profile.
// Required for each history byte and for query -> value / value -> query calls.
// The composite dispatcher must cover every profile callbacks can publish.
class NativeInputHistoryHost {
public:
    virtual ~NativeInputHistoryHost() = default;
    virtual std::uint8_t query_20(void* actual_device, std::uint32_t code) = 0;
    virtual float value_24(void* actual_device, std::uint32_t code) = 0;
};

// Borrow actual live image-value storage. No default constants are substituted;
// these reads retain their native timing and arithmetic precision.
struct NativeKeyboardMouseGlobals {
    volatile float& one_00d7a24c;
    volatile float& negative_zero_00d7a208;
    volatile float& mouse_scale_00e12fb0;
    volatile std::uint8_t& invert_y_00f8bc04;
    volatile double& axis_divisor_00d7a220;
    volatile float& unsigned_bias_00ce3978;
    volatile double& millisecond_divisor_00ce47a0;
};

struct NativeKeyboardMouseContext {
    Win32PlatformState* volatile& platform_0109cf04;
    NativeInputHistoryHost& dispatch;
    NativeKeyboardMouseGlobals globals;
};

// Complete native field operations over caller-owned, suitably aligned actual
// byte storage; not C++ InputDevice objects. The storage's first DWORD is the
// sole profile and +4 is the sole native reference count. Constructors preserve
// +8, padding, and the mouse sample's allocation preimage. Callers provide valid
// preimages for any field read before a later native write. No implicit new,
// AddRef, Release or host-side lifetime registration is performed here.
// A962F0: ECX storage, RET; A9A3E0/A9A290: ECX storage, stack DI8*, RET4.
void* construct_native_keyboard_base_00a962f0(void* storage) noexcept;
void* construct_native_keyboard_00a9a3e0(void* storage, IDirectInput8A&,
    NativeInputDeviceSdk&, NativeKeyboardMouseContext&);
void* construct_native_mouse_00a9a290(void* storage, IDirectInput8A&,
    NativeInputDeviceSdk&, NativeKeyboardMouseContext&);

// A95E60/A99EF0: ECX storage, tail BD30F0, RET; root profile becomes CEB130.
// Scalar entries RET4, bit0 frees singleton_lifetime_allocate-compatible storage
// through singleton_lifetime_free, matching actual enumeration's BF681B domain;
// the result is the captured allocation identity, even after freeing it.
// Native destruction and constructor EH do not release DirectInput references.
// SDK tracking must outlive all borrowers, followed by explicit host release.
void destroy_native_keyboard_base_00a95e60(void*) noexcept;
void destroy_native_mouse_base_00a99ef0(void*) noexcept;
void* delete_native_keyboard_00a9a470(void*, std::uint8_t flags) noexcept;
void* delete_native_mouse_00a9a390(void*, std::uint8_t flags) noexcept;

std::int32_t native_keyboard_class_00a96350() noexcept;
std::int32_t native_mouse_class_00a9a0f0() noexcept;
// A99E90 RET: previous-store, current-profile query, current-store per byte.
void update_native_input_history_00a99e90(void*, NativeInputHistoryHost&);
// Query/value use stack code, RET4; native keyboard code is unchecked. The raw
// caller must provide readable storage at +20C+code (normal keyboard range 0..255).
std::uint8_t query_native_keyboard_00a95e70(void*, std::uint32_t code) noexcept;
float value_native_keyboard_00a95e90(void*, std::uint32_t code,
    NativeKeyboardMouseContext&);
std::uint8_t query_native_mouse_00a99f70(void*, std::uint32_t code,
    NativeKeyboardMouseContext&);
float value_native_mouse_00a99fe0(void*, std::uint32_t code,
    NativeKeyboardMouseGlobals&);
// Poll ignores the original float stack argument and RET4. Real COM calls;
// preserved null/poll-failure arms, ignored HRESULTs and retained sample bytes.
bool poll_native_keyboard_00a9a4a0(void*, NativeKeyboardMouseContext&);
bool poll_native_mouse_00a9a180(void*, NativeKeyboardMouseContext&);
// ECX mouse, stack flags, RET4; marks +234 even if the real SDK call fails.
void set_native_mouse_cooperative_level_00a9a140(void*, std::uint32_t flags,
    NativeKeyboardMouseContext&);

} // namespace bsp
