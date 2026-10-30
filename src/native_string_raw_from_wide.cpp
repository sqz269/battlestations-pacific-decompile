#include "bsp/native_string_raw_from_wide.hpp"
#include "bsp/native_string.hpp"

#include <climits>
#include <cstdint>
#include <cstring>
#include <intrin.h>

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4 && sizeof(std::uintptr_t) == 4);
static_assert(sizeof(std::uint32_t) == 4 && sizeof(std::uint16_t) == 2);
static_assert(CHAR_BIT == 8);

template<class T> T read_scalar(std::uint32_t address) noexcept {
    T value;
    _ReadWriteBarrier();
    std::memcpy(&value, reinterpret_cast<const void*>(
        static_cast<std::uintptr_t>(address)), sizeof value);
    _ReadWriteBarrier();
    return value;
}
template<class T> void write_scalar(std::uint32_t address, T value) noexcept {
    _ReadWriteBarrier();
    std::memcpy(reinterpret_cast<void*>(static_cast<std::uintptr_t>(address)),
        &value, sizeof value);
    _ReadWriteBarrier();
}
std::uint8_t read_byte(std::uint32_t address) noexcept {
    _ReadWriteBarrier();
    const auto value = *reinterpret_cast<const volatile unsigned char*>(
        static_cast<std::uintptr_t>(address));
    _ReadWriteBarrier();
    return value;
}
void write_byte(std::uint32_t address, std::uint8_t value) noexcept {
    _ReadWriteBarrier();
    *reinterpret_cast<volatile unsigned char*>(
        static_cast<std::uintptr_t>(address)) = value;
    _ReadWriteBarrier();
}
} // namespace

void* construct_narrow_from_wide_00436630(void* actual_header8,
    const void* actual_wide_source, NativeStringRawPoolContext& strings) {
    const auto source = static_cast<std::uint32_t>(
        reinterpret_cast<std::uintptr_t>(actual_wide_source));
    const auto destination = static_cast<std::uint32_t>(
        reinterpret_cast<std::uintptr_t>(actual_header8));
    write_scalar<std::uint32_t>(destination, 0u);
    write_scalar<std::uint32_t>(destination + 4u, 0u);

    const auto after_first = source + 2u;
    auto scan = source;
    std::uint16_t unit;
    do {
        unit = read_scalar<std::uint16_t>(scan);
        scan += 2u;
    } while (unit != 0u);
    const auto difference = scan - after_first;
    const auto requested = (difference >> 1u) |
        ((difference & 0x80000000u) != 0u ? 0x80000000u : 0u); // SAR1 bits
    resize_native_string_header_0041dd40(actual_header8, strings, requested, true);

    auto output = read_scalar<std::uint32_t>(destination + 4u);
    if (output != 0u) {
        auto input = source;
        std::uint8_t byte;
        do {
            byte = read_byte(input); // BYTE only, not another full-WORD scan
            write_byte(output, byte);
            output += 1u;
            input += 2u;
        } while (byte != 0u);
    }
    return actual_header8;
}
} // namespace bsp
