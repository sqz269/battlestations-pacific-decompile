#include "bsp/native_string_range_assignment.hpp"

#include "bsp/native_string.hpp"

#include <cstring>
#include <intrin.h>

namespace bsp {
namespace {

static_assert(sizeof(std::uintptr_t) == 4);
static_assert(sizeof(NativeString) == 8);

std::uint32_t address(const void* value) noexcept {
    return reinterpret_cast<std::uintptr_t>(value);
}

void* pointer(std::uint32_t value) noexcept {
    return reinterpret_cast<void*>(value);
}

std::uint32_t word(std::uint32_t base, std::uint32_t offset = 0) noexcept {
    std::uint32_t value;
    _ReadWriteBarrier();
    std::memcpy(&value, pointer(base + offset), sizeof value);
    _ReadWriteBarrier();
    return value;
}

std::uint32_t sar_three(std::uint32_t value) noexcept {
    return (value >> 3) | ((value & 0x80000000u) ? 0xe0000000u : 0u);
}

} // namespace

void* __cdecl assign_native_string_header_range_0048da50(const void* first,
    const void* last, void* destination, std::uint32_t, std::uint32_t,
    std::uint32_t, NativeStringRawPoolContext& strings) {
    auto current = address(first);
    const auto end = address(last);
    auto output = address(destination);
    while (current != end) {
        if (output != current) {
            const auto requested_length = word(current);
            resize_native_string_header_0041dd40(pointer(output), strings,
                requested_length, true);
            if (word(current) != 0) {
                const auto copied_length = word(output);
                const auto* source_data = pointer(word(current, 4));
                auto* destination_data = pointer(word(output, 4));
                // Retain those native header reads even for a zero-byte call;
                // only the accepted Source CRT zero-copy omission follows.
                if (copied_length != 0)
                    std::memmove(destination_data, source_data, copied_length);
            }
        }
        current += 8u;
        output += 8u;
    }
    return pointer(output);
}

void* __cdecl assign_native_string_range_dispatch_0048f400(const void* first,
    const void* last, void* destination, NativeStringRawPoolContext& strings) {
    const auto captured_first = address(first);
    const auto captured_last = address(last);
    const auto captured_destination = address(destination);
    assign_native_string_header_range_0048da50(first, last, destination,
        captured_destination, 0u, 0u, strings);
    const auto count = sar_three(captured_last - captured_first);
    return pointer(captured_destination + count * 8u);
}

} // namespace bsp
