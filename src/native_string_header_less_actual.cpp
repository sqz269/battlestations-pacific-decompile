#include "bsp/native_string_header_less_actual.hpp"
#include "bsp/native_crt_stricmp.hpp"

namespace bsp {
namespace {

static_assert(sizeof(void*) == 4 && sizeof(std::uint32_t) == 4,
    "00443D00 Source requires Win32 pointer and DWORD representations");

std::uint32_t read_current_word(const void* actual_header,
    std::uint32_t byte_offset) noexcept {
    const auto address = static_cast<std::uint32_t>(
        reinterpret_cast<std::uintptr_t>(actual_header)) + byte_offset;
    return *reinterpret_cast<const volatile std::uint32_t*>(
        static_cast<std::uintptr_t>(address));
}

} // namespace

bool __stdcall less_native_string_headers_with_callees_00443d00(
    const void* actual_left_header, const void* actual_right_header,
    const volatile std::int32_t& actual_locale_changed_0109de1c,
    NativeCrtStricmpCallees& actual_callees) {
    if (read_current_word(actual_left_header, 0) == 0) {
        return read_current_word(actual_right_header, 0) != 0;
    }
    if (read_current_word(actual_right_header, 0) == 0) {
        return false;
    }

    // Native 00443D35 reads right.DATA4 before left.DATA4.
    const auto* const right_data = reinterpret_cast<const char*>(
        static_cast<std::uintptr_t>(read_current_word(actual_right_header, 4)));
    const auto* const left_data = reinterpret_cast<const char*>(
        static_cast<std::uintptr_t>(read_current_word(actual_left_header, 4)));
    return compare_native_crt_strings_00bf7fbf(left_data, right_data,
        actual_locale_changed_0109de1c, actual_callees) < 0;
}

} // namespace bsp
