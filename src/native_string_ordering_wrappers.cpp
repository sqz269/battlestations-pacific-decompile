#include "bsp/native_string_ordering_wrappers.hpp"

#include "bsp/native_vfs_date_leaf_providers.hpp"

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native string ordering wrappers require MSVC Win32.
#endif

namespace bsp {
namespace {

std::uint32_t read_word(const void* storage, std::uint32_t offset) noexcept {
    return *reinterpret_cast<const volatile std::uint32_t*>(
        reinterpret_cast<std::uintptr_t>(storage) + offset);
}

} // namespace

bool less_native_string_headers_00443cb0(
    const void* actual_left_header, const void* actual_right_header) {
    // The complete predicates are identical after their different native
    // argument setup. Reuse the accepted actual-header implementation.
    return less_native_string_headers_00443d00(actual_left_header, actual_right_header);
}

bool less_native_string_scalar_keys_0076eaa0(
    const void* actual_left_record, const void* actual_right_record) {
    // Native 0076EAA4..0076EACA inlines the same prefix predicate. Keep the
    // second, reversed comparison rather than caching the first result.
    if (less_native_string_headers_00443cb0(actual_left_record, actual_right_record))
        return true;
    if (less_native_string_headers_00443cb0(actual_right_record, actual_left_record))
        return false; // 0076EACF
    const auto left_high = read_word(actual_left_record, 0x0c); // 0076EAD8
    const auto right_high = read_word(actual_right_record, 0x0c); // 0076EADB
    if (left_high < right_high) return true;
    if (left_high > right_high) return false;
    const auto left_low = read_word(actual_left_record, 0x08); // 0076EAE2
    const auto right_low = read_word(actual_right_record, 0x08); // 0076EAE5
    return left_low < right_low;
}

} // namespace bsp
