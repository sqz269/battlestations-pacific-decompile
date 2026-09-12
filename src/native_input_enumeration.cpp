#include "bsp/native_input_enumeration.hpp"
#include "bsp/input_enumeration.hpp"
#include "bsp/singleton_lifetime.hpp"

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <cstddef>
#include <cstring>
#include <exception>

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4);
static_assert(sizeof(GUID) == 16);
static_assert(sizeof(NativeString) == 8);
static_assert(offsetof(DIDEVICEINSTANCEA, guidInstance) == 4);
static_assert(offsetof(DIDEVICEINSTANCEA, dwDevType) == 0x24);
static_assert(offsetof(DIDEVICEINSTANCEA, tszProductName) == 0x12c);

template<class T> T read(const void* p, std::size_t offset) noexcept {
    T value; std::memcpy(&value, static_cast<const std::byte*>(p) + offset, sizeof value);
    return value;
}
template<class T> void write(void* p, std::size_t offset, T value) noexcept {
    std::memcpy(static_cast<std::byte*>(p) + offset, &value, sizeof value);
}
void* at(void* p, std::size_t offset) noexcept {
    return static_cast<std::byte*>(p) + offset;
}
std::uint32_t guid_count(const void* header) noexcept {
    const auto begin = read<std::uint32_t>(header, 4);
    if (begin == 0) return 0;
    const auto bytes = read<std::uint32_t>(header, 8) - begin;
    return static_cast<std::uint32_t>(static_cast<std::int32_t>(bytes) >> 4);
}
bool same_guid(const void* first, const GUID& second) noexcept {
    // A972E0 has no callbacks or writes. Reuse its existing complete comparison;
    // these two call-local values are neither GUID storage nor device ownership.
    InputInstanceGuid a, b;
    std::memcpy(a.data(), first, sizeof(GUID));
    std::memcpy(b.data(), &second, sizeof(GUID));
    return input_instance_guids_equal_00a972e0(a, b);
}
void* allocate_device(std::size_t bytes) {
    return singleton_lifetime_allocate({SingletonAllocationKind::object, bytes, bytes});
}
void create_and_attach(void* backend, std::uint32_t slot, std::size_t bytes,
    const DIDEVICEINSTANCEA& instance, NativeInputEnumerationContext& c) {
    void* const allocation = allocate_device(bytes);
    void* constructed = nullptr;
    try {
        if (allocation) {
            auto* const direct_input = read<IDirectInput8A*>(backend, 0xe0);
            if (bytes == 0x310)
                constructed = c.calls.construct_keyboard_00a9a3e0(allocation, direct_input);
            else if (bytes == 0x23c)
                constructed = c.calls.construct_mouse_00a9a290(allocation, direct_input);
            else
                constructed = c.calls.construct_joystick_00a99940(allocation, direct_input, instance);
        }
    } catch (...) {
        // DECE38 states0/1 -> -1, state3 ->2: captured allocation free.
        // The constructor performs its own member unwind first.
        singleton_lifetime_free(allocation);
        throw;
    }
    // Native clears the allocation-cleanup state BEFORE this call. An attach
    // failure does not delete/free the returned device or reverse its effects.
    attach_native_input_device_00a904e0(backend, slot, constructed, c.calls);
}
struct EnumerationFrame {
    void* backend;
    NativeInputEnumerationContext& context;
    std::exception_ptr failure;
    EnumerationFrame* previous;
};
thread_local EnumerationFrame* enumeration_frame = nullptr;
struct FrameScope {
    EnumerationFrame frame;
    FrameScope(void* backend, NativeInputEnumerationContext& c)
        : frame{backend, c, {}, enumeration_frame} { enumeration_frame = &frame; }
    ~FrameScope() { enumeration_frame = frame.previous; }
};
} // namespace

void attach_native_input_device_00a904e0(void* backend, std::uint32_t slot,
    void* device, NativeInputEnumerationCalls& calls) {
    const auto profile = read<std::uint32_t>(device, 0);
    const auto device_class = calls.device_class_vslot08(device, profile);
    if (slot == 0xffffffffu) {
        slot = 0;
        while (slot < 8 && read<void*>(backend, 4 + device_class * 0x20u + slot * 4u))
            ++slot;
    }
    write(device, 8, slot);
    write(backend, 4 + (slot + device_class * 8u) * 4u, device);
}

int on_native_input_device_enumerated_00a98030(void* backend,
    const DIDEVICEINSTANCEA& instance, NativeInputEnumerationContext& c) {
    const auto type = instance.dwDevType & 0xffu;
    if (type == DI8DEVTYPE_KEYBOARD) {
        if (!read<void*>(backend, 4)) create_and_attach(backend, 0, 0x310, instance, c);
        return DIENUM_CONTINUE;
    }
    if (type == DI8DEVTYPE_MOUSE) {
        if (!read<void*>(backend, 0x24)) create_and_attach(backend, 0, 0x23c, instance, c);
        return DIENUM_CONTINUE;
    }
    if (type != DI8DEVTYPE_JOYSTICK && type != DI8DEVTYPE_GAMEPAD)
        return DIENUM_CONTINUE;

    NativeString product;
    product.assign_0041e870(c.strings, instance.tszProductName);
    // The native EBX capture survives all callbacks; normal cleanup releases
    // this pointer using the then-current local string length, without reset.
    char* const captured_product = product.data();
    try {
        const bool xbox = captured_product &&
            (std::strstr(captured_product, "Xbox") || std::strstr(captured_product, "XBOX") ||
             std::strstr(captured_product, "XBox")) && std::strstr(captured_product, "360");
        if (xbox) write<std::uint8_t>(backend, 0xf4, 1);
        void* const header = at(backend, 0xe4);
        std::uint32_t index = 0;
        for (; index < guid_count(header); ++index) {
            if (!read<void*>(header, 4) || index >= guid_count(header))
                c.calls.invalid_parameter_00bf6713();
            const auto begin = read<std::uint32_t>(header, 4);
            const auto element = begin + index * 16u;
            if (same_guid(reinterpret_cast<const void*>(element), instance.guidInstance)) break;
        }
        if (index == guid_count(header)) {
            c.calls.append_guid_00a97fa0(header, instance.guidInstance);
            if (!xbox && !read<void*>(backend, 0x44 + index * 4u))
                create_and_attach(backend, index, 0xb48, instance, c);
        }
    } catch (...) {
        // DECE38 state2 -> -1, CB69F6 -> actual8h destructor41DD20.
        destroy_native_string_header_0041dd20(&product, c.strings);
        throw;
    }
    if (captured_product) c.strings.release(captured_product, product.length() + 1u);
    return DIENUM_CONTINUE;
}

int __stdcall native_input_enum_callback_00a982b0(const DIDEVICEINSTANCEA* instance,
    void* backend) noexcept {
    auto* const frame = enumeration_frame;
    if (!frame || frame->backend != backend || frame->failure) return DIENUM_STOP;
    try {
        return on_native_input_device_enumerated_00a98030(backend, *instance, frame->context);
    } catch (...) {
        frame->failure = std::current_exception();
        return DIENUM_STOP;
    }
}

std::int32_t enumerate_native_input_devices(IDirectInput8A& direct_input, std::uint32_t type,
    void* backend, std::uint32_t flags, NativeInputEnumerationContext& c) {
    FrameScope scope(backend, c);
    const auto result = direct_input.EnumDevices(type, &native_input_enum_callback_00a982b0,
        backend, flags);
    if (scope.frame.failure) std::rethrow_exception(scope.frame.failure);
    return result;
}
} // namespace bsp
