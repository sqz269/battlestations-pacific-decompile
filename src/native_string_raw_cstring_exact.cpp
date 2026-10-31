#include "bsp/native_string_raw_cstring_exact.hpp"

#include "bsp/native_crt_memcpy.hpp"
#include "bsp/native_string_raw_resize_exact.hpp"

#include <climits>
#include <cstring>
#include <intrin.h>

namespace bsp {
namespace {
static_assert(CHAR_BIT == 8 && sizeof(std::uint8_t) == 1);
static_assert(sizeof(std::uint32_t) == 4);
static_assert(sizeof(void*) == 4 && sizeof(std::uintptr_t) == 4);

std::uint32_t address_bits(const void* base, std::uint32_t offset) noexcept {
    return static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(base))
        + offset;
}

void* pointer_at(std::uint32_t address) noexcept {
    return reinterpret_cast<void*>(static_cast<std::uintptr_t>(address));
}

std::uint8_t read_byte(std::uint32_t address) noexcept {
    std::uint8_t value;
    _ReadWriteBarrier();
    std::memcpy(&value, pointer_at(address), sizeof value);
    _ReadWriteBarrier();
    return value;
}

std::uint32_t read_word(const void* header, std::uint32_t offset) noexcept {
    std::uint32_t value;
    _ReadWriteBarrier();
    std::memcpy(&value, pointer_at(address_bits(header, offset)), sizeof value);
    _ReadWriteBarrier();
    return value;
}

void write_word(void* header, std::uint32_t offset,
    std::uint32_t value) noexcept {
    _ReadWriteBarrier();
    std::memcpy(pointer_at(address_bits(header, offset)), &value, sizeof value);
    _ReadWriteBarrier();
}
} // namespace

void* construct_native_string_header_0041e870_exact(
    void* actual_header, const void* actual_cstring,
    NativeStringRawPoolContext& context,
    const volatile std::uint32_t& actual_feature_word_0109eea4) {
    const void* const captured_input = actual_cstring;
    void* const captured_header = actual_header;
    write_word(captured_header, 0u, 0u);
    write_word(captured_header, 4u, 0u);

    const std::uint32_t source_start = address_bits(captured_input, 0u);
    std::uint32_t cursor = source_start;
    std::uint8_t captured_byte;
    do {
        captured_byte = read_byte(cursor);
        cursor += 1u;
    } while (captured_byte != 0u);
    const std::uint32_t request = cursor - source_start - 1u;

    std::uint32_t preserve_home = 1u;
    resize_native_string_header_0041dd40_exact(captured_header, context,
        request, &preserve_home, actual_feature_word_0109eea4);

    const std::uint32_t data = read_word(captured_header, 4u);
    if (data != 0u) {
        const std::uint32_t length = read_word(captured_header, 0u);
        const std::uint32_t count = length + 1u;
        copy_native_crt_bytes_00bf7680(pointer_at(data), captured_input, count,
            actual_feature_word_0109eea4);
    }
    return captured_header;
}

} // namespace bsp
