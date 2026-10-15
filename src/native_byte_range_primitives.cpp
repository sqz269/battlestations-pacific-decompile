#include "bsp/native_byte_range_primitives.hpp"

#include <cstring>

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4);
static_assert(sizeof(std::uintptr_t) == sizeof(std::uint32_t));

std::uint32_t address(const void* pointer) noexcept {
    return static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(pointer));
}

void* pointer(std::uint32_t value) noexcept {
    return reinterpret_cast<void*>(static_cast<std::uintptr_t>(value));
}

void fill_byte(std::uint32_t destination, const void* value) noexcept {
    // Both accesses are explicit: never turn this alias-sensitive operation
    // into a cached value or memset, even when value aliases the output.
    const auto byte = *static_cast<const volatile unsigned char*>(value);
    *reinterpret_cast<volatile unsigned char*>(static_cast<std::uintptr_t>(destination)) = byte;
}
} // namespace

void* __stdcall fill_n_native_byte_range_0041f450(
    void* first, std::uint32_t count, const void* value) {
    const auto begin = address(first);
    auto cursor = begin;
    auto remaining = count;
    while (remaining != 0) {
        fill_byte(cursor, value);
        --remaining;
        ++cursor;
    }
    return pointer(begin + count);
}

void __cdecl fill_native_byte_range_0041c630(
    void* first, void* last, const void* value) {
    auto cursor = address(first);
    const auto end = address(last);
    while (cursor != end) {
        fill_byte(cursor, value);
        ++cursor;
    }
}

void* __cdecl copy_backward_native_byte_range_0041c650(
    void* first, void* last, void* destination_end) {
    const auto count = address(last) - address(first);
    void* const result = pointer(address(destination_end) - count);
    // Native TEST/JLE: zero and every DWORD with bit31 set skip the call.
    if (count != 0 && count < 0x80000000U)
        (void)::memmove_s(result, count, first, count);
    return result;
}

void* __stdcall copy_native_byte_range_0041e630(
    void* first, void* last, void* destination) {
    const auto count = address(last) - address(first);
    void* const result = pointer(address(destination) + count);
    if (count != 0)
        (void)::memmove_s(destination, count, first, count);
    return result;
}
} // namespace bsp
