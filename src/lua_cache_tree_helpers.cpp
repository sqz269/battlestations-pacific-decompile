#include "bsp/lua_cache_tree_helpers.hpp"

#include "bsp/native_invalid_parameter_noinfo_call.hpp"
#include "bsp/native_string.hpp"

#include <cstring>

namespace bsp {
namespace {

static_assert(sizeof(std::uintptr_t) == 4);
static_assert(sizeof(NativeString) == 8);

std::uint32_t address(const void* value) noexcept {
    return reinterpret_cast<std::uintptr_t>(value);
}

const void* pointer(std::uint32_t value) noexcept {
    return reinterpret_cast<const void*>(value);
}

std::uint32_t word(std::uint32_t base, std::uint32_t offset = 0) noexcept {
    return *reinterpret_cast<const volatile std::uint32_t*>(base + offset);
}

void put(std::uint32_t base, std::uint32_t offset, std::uint32_t value) noexcept {
    *reinterpret_cast<volatile std::uint32_t*>(base + offset) = value;
}

std::uint32_t sar_three(std::uint32_t value) noexcept {
    // Exact DWORD SAR, including wrapped/negative pointer differences.
    return (value >> 3) | ((value & 0x80000000u) ? 0xe0000000u : 0u);
}

} // namespace

void* __cdecl mismatch_native_string_range_00b6c480(void* output,
    const void* first, const void* last, const void* second_first,
    std::uint32_t, std::uint32_t) {
    auto current = address(first);
    const auto end = address(last);
    auto second = address(second_first);
    while (current != end) {
        const auto left_length = word(current);
        const auto right_length = word(second);
        if (left_length != right_length) break;
        if (left_length != 0) {
            const auto* right_data = static_cast<const char*>(pointer(word(second, 4)));
            const auto* left_data = static_cast<const char*>(pointer(word(current, 4)));
            if (_stricmp(left_data, right_data) != 0) break;
        }
        current += 8;
        second += 8;
    }
    put(address(output), 4, second);
    put(address(output), 0, current);
    return output;
}

bool __cdecl equal_native_string_range_00b6c7b0(const void* first,
    const void* last, const void* second_owner, const void* second_first,
    std::uint32_t, std::uint32_t) {
    const auto count = sar_three(address(last) - address(first));
    const auto owner = address(second_owner);
    if (owner == 0) invoke_native_invalid_parameter_00bf6713();
    const auto second_end = address(second_first) + count * 8u;
    if (second_end > word(owner, 8) || second_end < word(owner, 4))
        invoke_native_invalid_parameter_00bf6713();

    // Native output aliases stack argument slots 3/4. Keep the result separate
    // in this source interface; no external header is written by this wrapper.
    std::uint32_t mismatch[2] = {owner, address(second_first)};
    const auto* result = static_cast<const std::uint32_t*>(
        mismatch_native_string_range_00b6c480(mismatch, first, last, second_first,
            address(first), address(last) & 0xffffff00u));
    return *result == address(last);
}

std::uint32_t __fastcall equal_native_string_vectors_00b6c990(
    const void* left, const void* right) {
    const auto left_owner = address(left);
    const auto right_owner = address(right);
    const auto left_begin_for_count = word(left_owner, 4);
    const auto left_count = left_begin_for_count == 0 ? 0u
        : sar_three(word(left_owner, 8) - left_begin_for_count);
    const auto right_begin = word(right_owner, 4);
    const auto right_count = right_begin == 0 ? 0u
        : sar_three(word(right_owner, 8) - right_begin);
    if (left_count != right_count) return 0;

    if (right_begin > word(right_owner, 8))
        invoke_native_invalid_parameter_00bf6713();
    const auto left_end = word(left_owner, 8);
    if (word(left_owner, 4) > left_end)
        invoke_native_invalid_parameter_00bf6713();
    const auto left_begin = word(left_owner, 4);
    if (left_begin > word(left_owner, 8))
        invoke_native_invalid_parameter_00bf6713();

    return equal_native_string_range_00b6c7b0(pointer(left_begin), pointer(left_end),
        right, pointer(right_begin), left_end, 0u) ? 1u : 0u;
}

} // namespace bsp
