#include "bsp/native_input_preset_pair_copy_construct.hpp"

#include "bsp/native_crt_memcpy.hpp"
#include "bsp/native_string.hpp"

#include <cstddef>
#include <cstring>

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4);

std::uint32_t word(const void* storage, std::uint32_t byte_offset) {
    const auto address = static_cast<std::uint32_t>(
        reinterpret_cast<std::uintptr_t>(storage)) + byte_offset;
    return *reinterpret_cast<const volatile std::uint32_t*>(address);
}

void word(void* storage, std::uint32_t byte_offset, std::uint32_t value) {
    const auto address = static_cast<std::uint32_t>(
        reinterpret_cast<std::uintptr_t>(storage)) + byte_offset;
    *reinterpret_cast<volatile std::uint32_t*>(address) = value;
}

void copy_mapped_word(void* destination, const void* source,
    std::uint32_t byte_offset) {
    const auto source_address = static_cast<std::uint32_t>(
        reinterpret_cast<std::uintptr_t>(source)) + byte_offset;
    const auto destination_address = static_cast<std::uint32_t>(
        reinterpret_cast<std::uintptr_t>(destination)) + byte_offset;
    std::byte representation[4];
    std::memcpy(representation,
        reinterpret_cast<const void*>(source_address), sizeof representation);
    std::memcpy(reinterpret_cast<void*>(destination_address),
        representation, sizeof representation);
}
} // namespace

void* __fastcall copy_construct_native_input_preset_pair_0069e7a0(
    void* actual_destination, std::uint32_t /*unused_edx*/,
    const void* actual_source, NativeStringRawPoolContext& strings,
    const volatile std::uint32_t& actual_feature_word_0109eea4) {
    // Keep the native comparison before the first volatile header reset.
    const volatile bool different = actual_destination != actual_source;
    word(actual_destination, 0, 0);
    word(actual_destination, 4, 0);
    if (different) {
        const auto requested_length = word(actual_source, 0);
        resize_native_string_header_0041dd40(
            actual_destination, strings, requested_length, true);
        if (word(actual_source, 0) != 0) {
            const auto copy_length = word(actual_destination, 0);
            const auto source_data = word(actual_source, 4);
            const auto destination_data = word(actual_destination, 4);
            copy_native_crt_bytes_00bf7680(
                reinterpret_cast<void*>(destination_data),
                reinterpret_cast<const void*>(source_data), copy_length,
                actual_feature_word_0109eea4);
        }
    }
    copy_mapped_word(actual_destination, actual_source, 0x08);
    copy_mapped_word(actual_destination, actual_source, 0x0c);
    copy_mapped_word(actual_destination, actual_source, 0x10);
    copy_mapped_word(actual_destination, actual_source, 0x14);
    copy_mapped_word(actual_destination, actual_source, 0x18);
    return actual_destination;
}
} // namespace bsp
