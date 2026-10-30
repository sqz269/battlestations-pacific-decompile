#include "bsp/native_wide_string_raw_from_narrow.hpp"
#include "bsp/native_wide_string.hpp"

#include <climits>
#include <cstdint>
#include <cstring>
#include <intrin.h>

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4 && sizeof(std::uintptr_t) == 4);
static_assert(sizeof(std::uint32_t) == 4 && sizeof(std::uint16_t) == 2);
static_assert(CHAR_BIT == 8);

std::uint8_t read_byte(std::uint32_t address) noexcept {
    _ReadWriteBarrier();
    const auto value = *reinterpret_cast<const volatile unsigned char*>(
        static_cast<std::uintptr_t>(address));
    _ReadWriteBarrier();
    return value;
}
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
} // namespace

void* construct_native_wide_string_header_004c5e60(void* actual_header8,
    const char* actual_source, NativeStringRawPoolContext& strings) {
    const auto source = static_cast<std::uint32_t>(
        reinterpret_cast<std::uintptr_t>(actual_source));
    const auto destination = static_cast<std::uint32_t>(
        reinterpret_cast<std::uintptr_t>(actual_header8));
    write_scalar<std::uint32_t>(destination, 0u);
    write_scalar<std::uint32_t>(destination + 4u, 0u);

    const auto after_first = source + 1u;
    auto scan = source;
    std::uint8_t byte;
    do {
        byte = read_byte(scan);
        scan += 1u;
    } while (byte != 0u);
    resize_native_wide_string_header_004c53e0(
        actual_header8, strings, scan - after_first, true);

    auto output = read_scalar<std::uint32_t>(destination + 4u);
    if (output != 0u) {
        auto input = source;
        do {
            byte = read_byte(input);
            write_scalar<std::uint16_t>(output, static_cast<std::uint16_t>(byte));
            output += 2u;
            input += 1u;
        } while (byte != 0u);
    }
    return actual_header8;
}
} // namespace bsp
