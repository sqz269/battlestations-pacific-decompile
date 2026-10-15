#include "bsp/native_string_vector_erase_unordered.hpp"

#include "bsp/native_string.hpp"
#include "bsp/native_string_vector.hpp"

#include <cstddef>
#include <cstdint>
#include <cstring>

namespace bsp {
namespace {

static_assert(sizeof(std::uintptr_t) == 4);
static_assert(sizeof(NativeString) == 8);
static_assert(sizeof(NativeStringVectorStorage) == 12);
static_assert(offsetof(NativeStringVectorStorage, data_00) == 0);
static_assert(offsetof(NativeStringVectorStorage, count_04) == 4);

std::uint32_t address(const void* value) noexcept {
    return reinterpret_cast<std::uintptr_t>(value);
}

void* pointer(std::uint32_t value) noexcept {
    return reinterpret_cast<void*>(value);
}

std::uint32_t word(std::uint32_t base, std::uint32_t offset = 0) noexcept {
    return *reinterpret_cast<const volatile std::uint32_t*>(base + offset);
}

void put(std::uint32_t base, std::uint32_t offset, std::uint32_t value) noexcept {
    *reinterpret_cast<volatile std::uint32_t*>(base + offset) = value;
}

} // namespace

void __fastcall erase_native_string_vector_unordered_0043ea00(
    void* actual_vector, NativeStringRawPoolContext& strings,
    const void* iterator_cell) {
    const auto vector_address = address(actual_vector);
    const auto captured_count = word(vector_address, 4);
    const auto captured_data = word(vector_address);
    const auto destination = word(address(iterator_cell));
    const auto captured_last = captured_data + captured_count * 8u - 8u;
    if (destination != captured_last) {
        // Both native calculations use the same captured count/data registers.
        const auto assignment_source = captured_data + captured_count * 8u - 8u;
        if (destination != assignment_source) {
            const auto requested_length = word(assignment_source);
            resize_native_string_header_0041dd40(pointer(destination), strings,
                requested_length, true);
            if (word(assignment_source) != 0) {
                const auto copied_length = word(destination);
                const auto* source_data = pointer(word(assignment_source, 4));
                auto* destination_data = pointer(word(destination, 4));
                // Preserve native reads even when the accepted Source CRT
                // zero-copy policy omits the call with a zero byte count.
                if (copied_length != 0)
                    std::memmove(destination_data, source_data, copied_length);
            }
        }
    }

    const auto destruction_count = word(vector_address, 4);
    const auto destruction_data = word(vector_address);
    const auto destruction_last = destruction_data + destruction_count * 8u - 8u;
    // This accepted body is precisely the native inline nonnull-data return:
    // capture data, then length+1; getter, return block; leave header untouched.
    destroy_native_string_header_0041dd20(pointer(destruction_last), strings);
    put(vector_address, 4, word(vector_address, 4) - 1u);
}

} // namespace bsp
