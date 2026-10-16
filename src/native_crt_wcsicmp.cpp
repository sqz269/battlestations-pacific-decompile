#include "bsp/native_crt_wcsicmp.hpp"

namespace bsp {

static_assert(sizeof(char16_t) == 2);
static_assert(sizeof(void*) == 4 && sizeof(std::uintptr_t) == 4,
    "Original CRT cursors use Win32 addresses");

std::int32_t compare_native_crt_wide_strings_00c03a39(
    const volatile std::int32_t& actual_locale_changed_0109de1c,
    NativeCrtWcsicmpServices& services, const char16_t* actual_left,
    const char16_t* actual_right) {
    if (actual_locale_changed_0109de1c != 0) {
        return services.compare_locale_00c0392a(services.context,
            actual_left, actual_right, nullptr);
    }
    if (actual_left == nullptr || actual_right == nullptr) {
        auto* const current_errno = services.errno_address_00bffb8b(services.context);
        *current_errno = 0x16;
        services.invalid_parameter_00bf66ef(services.context, 0, 0, 0, 0, 0);
        return 0x7fffffff;
    }

    auto left = reinterpret_cast<std::uintptr_t>(actual_left);
    auto right = reinterpret_cast<std::uintptr_t>(actual_right);
    for (;;) {
        std::uint32_t left_word = *reinterpret_cast<const volatile char16_t*>(left);
        if (left_word >= 0x41u && left_word <= 0x5au) left_word += 0x20u;
        std::uint32_t right_word = *reinterpret_cast<const volatile char16_t*>(right);
        if (right_word >= 0x41u && right_word <= 0x5au) right_word += 0x20u;
        left += 2u;
        right += 2u;

        if (left_word == 0) {
            return static_cast<std::int32_t>(left_word)
                - static_cast<std::int32_t>(right_word);
        }
        if (left_word != right_word) {
            return static_cast<std::int32_t>(left_word)
                - static_cast<std::int32_t>(right_word);
        }
    }
}

} // namespace bsp
