#include "bsp/native_input_guid_storage.hpp"

#include "bsp/native_singleton_vector_allocation.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <guiddef.h>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <new>

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4 && sizeof(_GUID) == 16);
constexpr std::uint32_t element_bytes = 16;
constexpr std::uint32_t max_elements = 0x0fffffffu;

std::uint32_t word(const void* header, std::size_t offset) noexcept {
    std::uint32_t result;
    std::memcpy(&result, static_cast<const std::byte*>(header) + offset, sizeof(result));
    return result;
}
void word(void* header, std::size_t offset, std::uint32_t value) noexcept {
    std::memcpy(static_cast<std::byte*>(header) + offset, &value, sizeof(value));
}
std::uint32_t elements(std::uint32_t first, std::uint32_t last) noexcept {
    return first ? static_cast<std::uint32_t>(static_cast<std::int32_t>(last - first) >> 4) : 0;
}
void validate_result(void* header, std::uint32_t insertion_index) {
    const auto captured_begin = word(header, 4);
    if (captured_begin > word(header, 8)) _invalid_parameter_noinfo();
    const auto position = captured_begin + insertion_index * element_bytes;
    if (position > word(header, 8) || position < word(header, 4))
        _invalid_parameter_noinfo();
}
} // namespace

void append_input_guid_storage(void* header, const _GUID& value) {
    const auto first = word(header, 4);
    const auto count = elements(first, word(header, 8));
    if (first && count < elements(first, word(header, 0xc))) {
        const auto destination = word(header, 8);
        // Complete old elements do not overlap the unoccupied end element.
        std::memcpy(reinterpret_cast<void*>(destination), &value, element_bytes);
        word(header, 8, destination + element_bytes);
        return;
    }

    const auto captured_position = word(header, 8);
    if (first > captured_position) _invalid_parameter_noinfo();
    const auto captured_begin = word(header, 4);
    std::uint32_t insertion_index = 0;
    if (elements(captured_begin, word(header, 8)) != 0) {
        if (captured_begin > word(header, 8)) _invalid_parameter_noinfo();
        insertion_index = elements(captured_begin, captured_position);
    }

    _GUID captured_value;
    std::memcpy(&captured_value, &value, element_bytes);
    const auto old_begin = word(header, 4);
    const auto old_capacity = elements(old_begin, word(header, 0xc));
    const auto old_count = elements(old_begin, word(header, 8));
    if (max_elements - old_count < 1u) native_singleton_length_error_00bd0590();

    const auto required = old_count + 1u;
    auto capacity = old_capacity;
    if (capacity < required) {
        capacity = max_elements - (old_capacity >> 1) < old_capacity
            ? 0u : old_capacity + (old_capacity >> 1);
        if (capacity < required) capacity = required;
    }
    if (capacity > max_elements) throw std::bad_alloc();
    const auto bytes = capacity * element_bytes;
    auto* const replacement = static_cast<std::byte*>(singleton_lifetime_allocate({
        SingletonAllocationKind::object, bytes, bytes}));

    // This host adapter implements append only. No middle-insert iterator or
    // element constructor is introduced. These memory copies cannot throw C++
    // exceptions; hardware faults are outside the declared valid-storage domain.
    if (old_count != 0)
        std::memcpy(replacement, reinterpret_cast<const void*>(old_begin), old_count * element_bytes);
    std::memcpy(replacement + old_count * element_bytes, &captured_value, element_bytes);

    const auto current_begin = word(header, 4);
    const auto retained_count = elements(current_begin, word(header, 8));
    if (current_begin) singleton_lifetime_free(reinterpret_cast<void*>(current_begin));
    const auto new_begin = reinterpret_cast<std::uint32_t>(replacement);
    word(header, 0xc, new_begin + bytes);
    word(header, 8, new_begin + (retained_count + 1u) * element_bytes);
    word(header, 4, new_begin);
    validate_result(header, insertion_index);
}

} // namespace bsp
