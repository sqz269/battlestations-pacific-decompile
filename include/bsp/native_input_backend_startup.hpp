#pragma once

#include "bsp/native_input_backend_owner.hpp"
#include <cstdint>

namespace bsp {
class NativeInputDeviceRuntime;

// Required raw activation service. The implementation is the separately
// recovered A91620 binding operation over this same F8h backend and its vectors.
class NativeInputBackendSlotActivation {
public:
    virtual ~NativeInputBackendSlotActivation() = default;
    virtual void activate_slot_00a91620(void* actual_backend,
        std::uint32_t device_class, std::uint32_t fixed_slot) = 0;
};

// Current-profile native virtual calls, without typed device/table projections.
// Poll's result is ignored by A918A0. These calls may mutate backend slots,
// requested counts, +64, profiles and later devices; the schedule rereads them.
class NativeInputBackendStartupDeviceCalls {
public:
    virtual ~NativeInputBackendStartupDeviceCalls() = default;
    virtual void reset_device_vslot14(void*) = 0;
    virtual void poll_device_vslot10(void*, float seconds) = 0;
    virtual std::uint8_t activity_device_vslot28(void*) = 0;
};

// Concrete source adapter to the existing SDK-backed finite raw device runtime.
// Borrows it; no allocation, singleton, device array or SDK owner is introduced.
class NativeInputBackendStartupDeviceRuntime final : public NativeInputBackendStartupDeviceCalls {
public:
    explicit NativeInputBackendStartupDeviceRuntime(NativeInputDeviceRuntime&) noexcept;
    void reset_device_vslot14(void*) override;
    void poll_device_vslot10(void*, float seconds) override;
    std::uint8_t activity_device_vslot28(void*) override;
private:
    NativeInputDeviceRuntime& runtime_;
};

// A900F0: ECX raw backend, no stack args, RET. All 24 owning slots +4..+60
// are inspected in class/slot order. A nonnull slot is reloaded before reset.
void reset_native_input_backend_00a900f0(void*, NativeInputBackendStartupDeviceCalls&);

// A90490: ECX backend, class stack, RET4/AL0-or1. The active vector is
// backend+6C+class*24h, requested DWORD at+68+class*24h. Null begin means0
// without reading end. Native DWORD subtraction/SAR2 and equality are retained.
// Original callers supply class0..2; no added bounds or malformed-span guard.
bool native_input_backend_at_requested_count_00a90490(const void*, std::uint32_t device_class) noexcept;

// A918A0: ECX backend, float seconds stack, RET4. Current backend slot10
// prepass, then 3x8 poll/count/activity/activation schedule. It retains the
// polled device for activity, while activation reloads the current slot itself.
// Source prepass admits D5B72C -> real A97390 RET and D5B5F8 -> CRT purecall.
void update_native_input_backend_00a918a0(void*, float seconds,
    NativeInputBackendStartupDeviceCalls&, NativeInputBackendSlotActivation&);

// Source outer-slot04 dispatch for callers that already captured the profile.
// Both D5B5F8 and D5B72C select A918A0, which separately captures its prepass.
void invoke_native_input_backend_update_vslot04(void*, std::uint32_t captured_profile,
    float seconds, NativeInputBackendStartupDeviceCalls&, NativeInputBackendSlotActivation&);

inline constexpr std::uint32_t native_input_startup_callback_identity = 0x004b4630;
// 4B4630 is a verified bare RET. Its +D8 consumers pass class in ECX and
// index in EDX, no stack words; removal passes -1. No semantic return value.
void native_input_startup_callback_004b4630(std::uint32_t device_class,
    std::int32_t index) noexcept;
// Call only after the consumer's native nonnull test; unknown identities are
// explicit binding errors. Never invoke an original numeric process address.
void invoke_native_input_startup_callback_004b4630(std::uint32_t captured_identity,
    std::uint32_t device_class, std::int32_t index);

// Source composition of ONLY 73DD6C..73DDB3, not a native73D410 replacement:
// existing F8h allocation/constructor, reload publication, callback+D8 write,
// reload publication, reset. Returns the allocated identity for source tracking;
// it is not substituted for either publication reload. After constructor
// completion, a reset exception leaves the registered allocation intact.
// All context/services and the genuine published backend must stay alive.
void* create_and_reset_native_input_backend(NativeInputBackendOwnerContext&,
    NativeInputBackendStartupDeviceCalls&);

// New C++ interfaces over valid raw storage. No original callable/FH3/SEH ABI,
// asynchronous mutation/hardware-fault compatibility or application wiring claim.
} // namespace bsp
