#include "bsp/native_input_preset_pair_construct.hpp"
#include "bsp/native_crt_memcpy.hpp"
#include "bsp/native_string.hpp"

#include <cstddef>
#include <cstring>

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4 && sizeof(Word) == 4);

Word word(const void* base, Word offset) {
    return *reinterpret_cast<const volatile Word*>(
        reinterpret_cast<std::uintptr_t>(base) + offset);
}
void put(void* base, Word offset, Word value) {
    *reinterpret_cast<volatile Word*>(
        reinterpret_cast<std::uintptr_t>(base) + offset) = value;
}
void* pointer(Word value) {
    return reinterpret_cast<void*>(static_cast<std::uintptr_t>(value));
}
} // namespace

void* __fastcall construct_native_input_preset_pair_0069e6e0(
    void* actual_destination, std::uint32_t unused_edx,
    const void* actual_key_header, const void* actual_mapped5,
    NativeStringRawPoolContext& strings,
    const volatile std::uint32_t& actual_feature_word_0109eea4) {
    static_cast<void>(unused_edx);
    void* const destination = actual_destination;
    const volatile bool self_key = destination == actual_key_header;
    put(destination, 0, 0);
    put(destination, 4, 0);
    if (!self_key) {
        const Word requested = word(actual_key_header, 0);
        resize_native_string_header_0041dd40(destination, strings, requested, true);
        if (word(actual_key_header, 0) != 0) {
            const Word count = word(destination, 0);
            const void* const source_data = pointer(word(actual_key_header, 4));
            void* const destination_data = pointer(word(destination, 4));
            copy_native_crt_bytes_00bf7680(destination_data, source_data, count,
                actual_feature_word_0109eea4);
        }
    }

    const auto* const mapped = static_cast<const std::byte*>(actual_mapped5);
    auto* const output = static_cast<std::byte*>(destination);
    std::byte mapped0[4];
    std::memcpy(mapped0, mapped, 4);
    std::memcpy(output + 8, mapped0, 4);
    std::byte mapped4[4];
    std::memcpy(mapped4, mapped + 4, 4);
    std::memcpy(output + 0xc, mapped4, 4);
    std::byte mapped8[4];
    std::memcpy(mapped8, mapped + 8, 4);
    std::memcpy(output + 0x10, mapped8, 4);
    std::byte mappedc[4];
    std::memcpy(mappedc, mapped + 0xc, 4);
    std::memcpy(output + 0x14, mappedc, 4);
    std::byte mapped10[4];
    std::memcpy(mapped10, mapped + 0x10, 4);
    std::memcpy(output + 0x18, mapped10, 4);
    return destination;
}
} // namespace bsp
