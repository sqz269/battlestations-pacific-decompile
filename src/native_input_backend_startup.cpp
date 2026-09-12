#include "bsp/native_input_backend_startup.hpp"
#include "bsp/native_input_device_runtime.hpp"
#include "bsp/input_focus_reset.hpp"

#include <cstdlib>
#include <exception>
#include <stdexcept>

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4, "native backend schedule requires Win32");
template<class T> T read(const void* p, std::uint32_t offset) noexcept {
    return *reinterpret_cast<const volatile T*>(static_cast<const std::byte*>(p) + offset);
}
template<class T> void write(void* p, std::uint32_t offset, T value) noexcept {
    *reinterpret_cast<volatile T*>(static_cast<std::byte*>(p) + offset) = value;
}
constexpr std::uint32_t groups_profile = 0x00d5b5f8;
constexpr std::uint32_t backend_profile = 0x00d5b72c;
[[noreturn]] void unsupported_profile() {
    throw std::invalid_argument("unbound native input backend profile");
}
void prepass(void* backend) {
    switch (read<std::uint32_t>(backend, 0)) {
    case backend_profile: input_backend_pre_tick_00a97390(); return;
    case groups_profile:
        (void)_purecall();
        std::terminate(); // The actual CRT purecall contract does not return.
    default: unsupported_profile();
    }
}
}

NativeInputBackendStartupDeviceRuntime::NativeInputBackendStartupDeviceRuntime(
    NativeInputDeviceRuntime& runtime) noexcept : runtime_(runtime) {}
void NativeInputBackendStartupDeviceRuntime::reset_device_vslot14(void* device) {
    runtime_.reset_device_vslot14(device);
}
void NativeInputBackendStartupDeviceRuntime::poll_device_vslot10(void* device, float seconds) {
    (void)runtime_.poll_device_vslot10(device, seconds);
}
std::uint8_t NativeInputBackendStartupDeviceRuntime::activity_device_vslot28(void* device) {
    return runtime_.activity_vslot28(device, read<std::uint32_t>(device, 0));
}

void reset_native_input_backend_00a900f0(void* backend, NativeInputBackendStartupDeviceCalls& calls) {
    for (std::uint32_t device_class = 0; device_class != 3; ++device_class) {
        for (std::uint32_t slot = 0; slot != 8; ++slot) {
            const auto offset = 4 + device_class * 0x20 + slot * 4;
            if (read<void*>(backend, offset))
                calls.reset_device_vslot14(read<void*>(backend, offset));
        }
    }
}

bool native_input_backend_at_requested_count_00a90490(const void* backend,
    std::uint32_t device_class) noexcept {
    const auto offset = 0x6c + device_class * 0x24;
    const auto begin = read<std::uint32_t>(backend, offset + 4);
    std::uint32_t count = 0;
    if (begin) {
        const auto difference = read<std::uint32_t>(backend, offset + 8) - begin;
        count = static_cast<std::uint32_t>(static_cast<std::int32_t>(difference) >> 2);
    }
    return read<std::uint32_t>(backend, 0x68 + device_class * 0x24) == count;
}

void update_native_input_backend_00a918a0(void* backend, float seconds,
    NativeInputBackendStartupDeviceCalls& calls, NativeInputBackendSlotActivation& activation) {
    prepass(backend);
    for (std::uint32_t device_class = 0; device_class != 3; ++device_class) {
        for (std::uint32_t slot = 0; slot != 8; ++slot) {
            void* const device = read<void*>(backend, 4 + device_class * 0x20 + slot * 4);
            if (!device) continue;
            // A918CC/D6: original stack seconds is loaded/stored as float for
            // EACH poll. Keep its x87 conversion instead of a copied bit word.
            float poll_seconds;
            __asm {
                fld seconds
                fstp poll_seconds
            }
            calls.poll_device_vslot10(device, poll_seconds);
            if (native_input_backend_at_requested_count_00a90490(backend, device_class)) continue;
            if (!read<std::uint8_t>(backend, 0x64) && device_class == 2 &&
                !calls.activity_device_vslot28(device)) continue;
            activation.activate_slot_00a91620(backend, device_class, slot);
        }
    }
}

void invoke_native_input_backend_update_vslot04(void* backend, std::uint32_t captured_profile,
    float seconds, NativeInputBackendStartupDeviceCalls& calls, NativeInputBackendSlotActivation& activation) {
    switch (captured_profile) {
    case backend_profile: case groups_profile:
        update_native_input_backend_00a918a0(backend, seconds, calls, activation); return;
    default: unsupported_profile();
    }
}

void native_input_startup_callback_004b4630(std::uint32_t, std::int32_t) noexcept {}
void invoke_native_input_startup_callback_004b4630(std::uint32_t captured_identity,
    std::uint32_t device_class, std::int32_t index) {
    if (captured_identity != native_input_startup_callback_identity)
        throw std::invalid_argument("unbound native input backend callback identity");
    native_input_startup_callback_004b4630(device_class, index);
}

void* create_and_reset_native_input_backend(NativeInputBackendOwnerContext& context,
    NativeInputBackendStartupDeviceCalls& calls) {
    void* const allocation = create_native_input_backend(context);
    void* const callback_owner = context.global_00f8bbf4;
    write(callback_owner, 0xd8, native_input_startup_callback_identity);
    void* const reset_owner = context.global_00f8bbf4;
    reset_native_input_backend_00a900f0(reset_owner, calls);
    return allocation;
}
} // namespace bsp
