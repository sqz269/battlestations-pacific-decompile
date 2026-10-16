#include "bsp/native_crt_ascii_stricmp.hpp"

namespace bsp {

static_assert(sizeof(void*) == 4 && sizeof(std::uintptr_t) == 4,
    "Original CRT cursor arithmetic uses Win32 addresses");

std::int32_t compare_native_crt_ascii_strings_00bf7eb7(
    const char* actual_left, const char* actual_right) noexcept {
    auto left = reinterpret_cast<std::uintptr_t>(actual_left);
    auto right = reinterpret_cast<std::uintptr_t>(actual_right);
    for (;;) {
        std::uint32_t left_byte =
            *reinterpret_cast<const volatile unsigned char*>(left);
        const auto left_range = left_byte - 0x41u;
        left += 1u;
        if (left_range <= 0x19u) left_byte += 0x20u;

        std::uint32_t right_byte =
            *reinterpret_cast<const volatile unsigned char*>(right);
        const auto right_range = right_byte - 0x41u;
        right += 1u;
        if (right_range <= 0x19u) right_byte += 0x20u;

        if (left_byte == 0) {
            return static_cast<std::int32_t>(left_byte)
                - static_cast<std::int32_t>(right_byte);
        }
        if (left_byte != right_byte) {
            return static_cast<std::int32_t>(left_byte)
                - static_cast<std::int32_t>(right_byte);
        }
    }
}

} // namespace bsp
