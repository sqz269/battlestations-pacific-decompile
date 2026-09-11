#include "bsp/native_string_compare.hpp"

#include "bsp/native_string.hpp"

#include <cstddef>
#include <cstring>

namespace bsp {
namespace {

static_assert(sizeof(NativeString) == 8);
static_assert(sizeof(std::uintptr_t) == 4);

template<class T> T read_header(const void* header, std::size_t offset) noexcept {
    T value;
    std::memcpy(&value, static_cast<const char*>(header) + offset, sizeof(value));
    return value;
}

} // namespace

bool equal_native_string_header_00425850(const void* actual_header,
    const char* candidate) noexcept {
    const auto* const data = read_header<const char*>(actual_header, 4);
    if (data == nullptr) {
        if (candidate == nullptr) return true;
        // The original scans through the terminator even when byte0 is not
        // zero. Volatile byte reads preserve that complete scan rather than
        // allowing strlen(candidate)==0 to collapse into a first-byte test.
        auto cursor = reinterpret_cast<std::uintptr_t>(candidate);
        const auto begin = cursor;
        std::uint8_t byte;
        do {
            byte = *reinterpret_cast<const volatile std::uint8_t*>(cursor);
            ++cursor;
        } while (byte != 0);
        return cursor - (begin + 1u) == 0;
    }
    if (candidate == nullptr)
        return read_header<std::uint32_t>(actual_header, 0) == 0;
    return _stricmp(data, candidate) == 0;
}

void replace_native_string_character_004259f0(void* actual_header,
    std::uint8_t old_byte, std::uint8_t new_byte, std::int32_t start) noexcept {
    if (read_header<char*>(actual_header, 4) == nullptr) return;
    std::uint32_t index;
    if (start < 0) {
        index = 0;
    } else {
        index = static_cast<std::uint32_t>(start);
        if (index > read_header<std::uint32_t>(actual_header, 0)) return;
    }
    if (index >= read_header<std::uint32_t>(actual_header, 0)) return;
    do {
        const auto data = read_header<std::uintptr_t>(actual_header, 4);
        auto* const current = reinterpret_cast<volatile std::uint8_t*>(data + index);
        if (*current == old_byte) *current = new_byte;
        ++index;
    } while (index < read_header<std::uint32_t>(actual_header, 0));
}

} // namespace bsp
