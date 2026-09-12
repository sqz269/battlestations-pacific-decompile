#pragma once

#include "bsp/native_gamepad_xinput.hpp"

#include <cstddef>
#include <cstdint>

namespace bsp {

// Actual allocation sizes, with no implicit byte initialization or C++ vtable.
// +0 is the sole native profile; +4 is a kind tag, NEVER a reference count.
struct alignas(4) NativeConstantForceRequestStorage { std::byte bytes[0x14]; };
struct alignas(4) NativeFadingForceRequestStorage { std::byte bytes[0x18]; };
struct alignas(4) NativeAlternatingForceRequestStorage { std::byte bytes[0x28]; };
static_assert(sizeof(NativeConstantForceRequestStorage) == 0x14);
static_assert(sizeof(NativeFadingForceRequestStorage) == 0x18);
static_assert(sizeof(NativeAlternatingForceRequestStorage) == 0x28);

inline constexpr std::uint32_t native_force_request_base_profile = 0x00d0db64;
inline constexpr std::uint32_t native_constant_force_request_profile = 0x00d0db78;
inline constexpr std::uint32_t native_fading_force_request_profile = 0x00d0db8c;
inline constexpr std::uint32_t native_alternating_force_request_profile = 0x00d0dba0;

// Complete standalone constructor bodies over valid caller-owned raw storage.
// Original ECX=this, EAX=this; constant/fading RET0C, alternating RET1C.
// Selector is one unnormalized byte. Alternating leaves +D..+F untouched.
void* construct_native_constant_force_request_008722d0(void*, std::uint32_t channel,
    float amplitude, float remaining) noexcept;
void* construct_native_fading_force_request_00872350(void*, std::uint32_t channel,
    float amplitude, float duration) noexcept;
void* construct_native_alternating_force_request_008723f0(void*, std::uint32_t channel,
    std::uint8_t selector, float first_value, float second_value,
    float first_period, float second_period, float duration) noexcept;

// Source allocation helpers use the BF681B-compatible singleton CRT domain.
// These accept already evaluated arguments; they do NOT reproduce event
// producers' post-allocation definition reloads or perform request submission.
void* allocate_native_constant_force_request(std::uint32_t channel,
    float amplitude, float remaining);
void* allocate_native_fading_force_request(std::uint32_t channel,
    float amplitude, float duration);
void* allocate_native_alternating_force_request(std::uint32_t channel,
    std::uint8_t selector, float first_value, float second_value,
    float first_period, float second_period, float duration);

// Original ECX=this, flag DWORD stack slot, EAX=captured allocation, RET4.
// All stamp D0DB64. Bit0 frees through singleton_lifetime_free; other bits do
// not free. No refcount, member owner, SDK release or device callback exists.
void* delete_native_force_request_base_008724c0(void*, std::uint32_t flags) noexcept;
void* delete_native_constant_force_request_008724e0(void*, std::uint32_t flags) noexcept;
void* delete_native_fading_force_request_00872500(void*, std::uint32_t flags) noexcept;
void* delete_native_alternating_force_request_00872520(void*, std::uint32_t flags) noexcept;

// Original ECX=this, RET; channel EAX, value ST0, expired EAX0/1.
std::uint32_t native_constant_force_request_channel_00872310(const void*) noexcept;
std::uint32_t native_fading_force_request_channel_008723a0(const void*) noexcept;
std::uint32_t native_alternating_force_request_channel_00872460(const void*) noexcept;
float native_constant_force_request_value_00872320(const void*) noexcept;
float native_fading_force_request_value_008723b0(const void*) noexcept;
float native_alternating_force_request_value_00872470(const void*) noexcept;
bool native_constant_force_request_expired_00872330(const void*) noexcept;
bool native_fading_force_request_expired_008723d0(const void*) noexcept;
bool native_alternating_force_request_expired_008724a0(const void*) noexcept;

// Original ECX=this, seconds stack argument, RET4. Borrow the actual live
// D7A278 double; do not substitute an immutable FLT_MAX literal.
void update_native_constant_force_request_00a93f90(void*, float seconds,
    const volatile double& infinite_remaining_00d7a278) noexcept;
void update_native_fading_force_request_00a93fc0(void*, float seconds) noexcept;
void update_native_alternating_force_request_00a94510(void*, float seconds);

// Implements exactly the five request methods of the existing raw consumer.
// Each method reloads the CURRENT profile at request+0. Base profile's four
// non-deleting slots call the real CRT _purecall; unknown profiles are binding
// errors. Concrete application subclasses still supply genuine device force
// and value operations inherited from NativeGamepadDispatch. No request map,
// device projection, shadow ownership, singleton, or synthetic device exists.
class NativeGamepadForceRequestDispatch : public NativeGamepadDispatch {
public:
    explicit NativeGamepadForceRequestDispatch(
        const volatile double& infinite_remaining_00d7a278) noexcept;
    void request_delete_vslot00(void*, std::uint32_t flags) final;
    std::uint32_t request_channel_vslot04(void*) final;
    float request_value_vslot08(void*) final;
    bool request_expired_vslot0c(void*) final;
    void request_update_vslot10(void*, float seconds) final;
private:
    const volatile double& infinite_remaining_;
};

// New C++ call interfaces: not original native vtable/exception ABI replacement.
// Valid nonnull raw storage is required; slot00(flags1) ends its ownership.
} // namespace bsp
