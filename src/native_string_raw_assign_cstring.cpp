#include "bsp/native_string_raw_assign_cstring.hpp"
#include "bsp/native_crt_memcpy.hpp"
#include "bsp/native_string.hpp"

#include <climits>
#include <cstring>
#include <intrin.h>

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4 && sizeof(std::uintptr_t) == 4);
static_assert(sizeof(std::uint32_t) == 4 && CHAR_BIT == 8);

std::uint8_t read_byte(std::uint32_t address) noexcept {
    _ReadWriteBarrier();
    const auto value = *reinterpret_cast<const volatile unsigned char*>(
        static_cast<std::uintptr_t>(address));
    _ReadWriteBarrier();
    return value;
}
std::uint32_t read_word(std::uint32_t address) noexcept {
    std::uint32_t value;
    _ReadWriteBarrier();
    std::memcpy(&value, reinterpret_cast<const void*>(
        static_cast<std::uintptr_t>(address)), sizeof value);
    _ReadWriteBarrier();
    return value;
}
} // namespace

void* assign_native_string_cstring_0041e350(void* actual_header8,
    const char* nullable_source, NativeStringRawPoolContext& strings,
    const volatile std::uint32_t& feature) {
    const char* const source = nullable_source;
    const auto source_address = static_cast<std::uint32_t>(
        reinterpret_cast<std::uintptr_t>(source));
    const auto destination = static_cast<std::uint32_t>(
        reinterpret_cast<std::uintptr_t>(actual_header8));
    std::uint32_t requested = 0;
    if (source != nullptr) {
        const auto after_first = source_address + 1u;
        auto scan = source_address;
        std::uint8_t byte;
        do {
            byte = read_byte(scan);
            scan += 1u;
        } while (byte != 0u);
        requested = scan - after_first;
    }
    resize_native_string_header_0041dd40(actual_header8, strings, requested, false);
    const auto data = read_word(destination + 4u);
    if (data != 0u) {
        const auto count = read_word(destination);
        // Native enters BF7680 even for count0 and a captured null source.
        copy_native_crt_bytes_00bf7680(
            reinterpret_cast<void*>(static_cast<std::uintptr_t>(data)),
            source, count, feature);
    }
    return actual_header8;
}
} // namespace bsp
