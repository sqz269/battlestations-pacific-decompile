#include "bsp/native_gamepad_rumble.hpp"
#include "bsp/native_int_pointer_tree18_leaves.hpp"

#include <cstdlib>

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4, "native rumble storage requires Win32");
void* at(void* p, std::uint32_t n) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(p) + n);
}
template<class T> T read(void* p, std::uint32_t n) noexcept {
    return *static_cast<const volatile T*>(at(p, n));
}
template<class T> void write(void* p, std::uint32_t n, T value) noexcept {
    *static_cast<volatile T*>(at(p, n)) = value;
}
std::uint32_t pointer_count(void* backend, std::uint32_t begin) noexcept {
    const auto bytes = read<std::uint32_t>(backend, 0xbc) - begin;
    return static_cast<std::uint32_t>(static_cast<std::int32_t>(bytes) >> 2);
}
bool greater(float value, float previous) noexcept {
    unsigned char result;
    __asm {
        fld previous
        fld value
        fcomip st(0), st(1)
        fstp st(0)
        seta result
    }
    return result != 0;
}
bool changed(float value, float previous) noexcept {
    unsigned char result;
    __asm {
        fld value
        fld previous
        fxch st(1)
        fucomip st(0), st(1)
        fstp st(0)
        lahf
        test ah, 44h
        setp result
    }
    return result != 0; // unequal or unordered; equal signed zero is unchanged
}
struct Iterator { void* owner; void* node; };
} // namespace

void refresh_native_gamepad_force_channel_00a949a0(void* device,
    std::uint32_t channel, NativeGamepadRumbleContext& c) {
    if (!c.enabled_00e12f2c) return;
    void* const tree = at(device, 0x20c);
    Iterator iterator{tree, read<void*>(read<void*>(device, 0x210), 0)};
    float maximum = 0.0f;
    for (;;) {
        void* const captured_head = read<void*>(tree, 4);
        if (!iterator.owner || iterator.owner != tree) _invalid_parameter_noinfo();
        if (iterator.node == captured_head) break;
        if (!iterator.owner) _invalid_parameter_noinfo();
        if (iterator.node == read<void*>(iterator.owner, 4)) _invalid_parameter_noinfo();
        // The virtual float result is rounded to binary32 BEFORE the second
        // validation and reloaded request/channel call (native A94A15 FSTP).
        const volatile float value = c.requests.request_value_vslot08(read<void*>(iterator.node, 0x10));
        if (iterator.node == read<void*>(iterator.owner, 4)) _invalid_parameter_noinfo();
        const auto current_channel =
            c.requests.request_channel_vslot04(read<void*>(iterator.node, 0x10));
        if (current_channel == channel && greater(value, maximum)) maximum = value;
        increment_native_int_pointer_tree18_00869a20(&iterator);
    }
    const auto amplitude_offset = 0x218u + channel * 4u;
    if (changed(maximum, read<float>(device, amplitude_offset))) {
        const auto captured_profile = read<std::uint32_t>(device, 0);
        write(device, amplitude_offset, maximum);
        c.output.set_force_vslot38(device, captured_profile, channel, maximum);
    }
}

void set_native_gamepad_rumble_enabled_00a94c50(bool enabled,
    NativeGamepadRumbleContext& c) {
    c.enabled_00e12f2c = enabled;
    void* backend = c.backend_00f8bbf4;
    const auto initial_begin = read<std::uint32_t>(backend, 0xb8);
    const auto initial_count = initial_begin == 0 ? 0 :
        static_cast<std::int32_t>(pointer_count(backend, initial_begin));
    for (std::int32_t index = 0; index < initial_count; ++index) {
        const auto first = read<std::uint32_t>(backend, 0xb8);
        if (first == 0 || static_cast<std::uint32_t>(index) >= pointer_count(backend, first))
            continue;
        const auto checked_begin = read<std::uint32_t>(backend, 0xb8);
        void* const captured_begin_slot = at(backend, 0xb8);
        if (checked_begin == 0 ||
            static_cast<std::uint32_t>(index) >= pointer_count(backend, checked_begin)) {
            _invalid_parameter_noinfo();
            backend = c.backend_00f8bbf4;
        }
        void* const device = read<void*>(read<void*>(captured_begin_slot, 0),
            static_cast<std::uint32_t>(index) * 4u);
        if (!device) continue;
        for (std::uint32_t channel = 0; channel != 2; ++channel) {
            if (c.enabled_00e12f2c) {
                refresh_native_gamepad_force_channel_00a949a0(device, channel, c);
            } else {
                const auto captured_profile = read<std::uint32_t>(device, 0);
                c.output.set_force_vslot38(device, captured_profile, channel, 0.0f);
                write(device, 0x218u + channel * 4u, 0.0f);
            }
        }
        backend = c.backend_00f8bbf4;
    }
}
} // namespace bsp
