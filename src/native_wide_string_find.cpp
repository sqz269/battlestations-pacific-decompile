#include "bsp/native_wide_string_find.hpp"
#include "bsp/native_crt_wcscspn.hpp"

#include <cstdint>
#include <cstring>
#include <intrin.h>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw wide first-of requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4 && sizeof(std::uintptr_t) == 4 &&
    sizeof(Word) == 4 && sizeof(std::int32_t) == 4 && sizeof(unsigned int) == 4);

Word word(const void* header, Word offset) noexcept {
    const auto address = reinterpret_cast<std::uintptr_t>(header) + offset;
    Word value;
    _ReadWriteBarrier();
    std::memcpy(&value, reinterpret_cast<const void*>(address), sizeof value);
    _ReadWriteBarrier();
    return value;
}
const void* pointer(Word bits) noexcept {
    return reinterpret_cast<const void*>(static_cast<std::uintptr_t>(bits));
}
} // namespace

std::int32_t find_first_of_native_wide_string_header_00a9ef60(
    const void* haystack, const void* set, std::int32_t start) {
    const Word set_data = word(set, 4u); // A9EF67, FIRST even if hay data NULL
    const Word hay_data = word(haystack, 4u); // A9EF6A
    if (hay_data == 0u) return -1;
    const Word length = word(haystack, 0u); // only with nonNULL hay data
    if (length == 0u) return -1;
    if (set_data == 0u) return -1;
    Word position = static_cast<Word>(start);
    if ((position & 0x80000000u) != 0u) position = 0u;
    else if (position > length) return -1; // equality still reaches CRT

    const Word scan = hay_data + position * 2u;
    const Word span = count_native_crt_wide_prefix_excluding_00c0382c(
        pointer(scan), pointer(set_data)); // genuine raw CRT, exactly ONCE
    const Word index = span + position;
    const Word current_length = word(haystack, 0u); // AFTER CRT, never captured length
    if (index == current_length) return -1; // equality ONLY
    std::int32_t result;
    std::memcpy(&result, &index, sizeof result); // exact signed DWORD return bits
    return result;
}
} // namespace bsp
