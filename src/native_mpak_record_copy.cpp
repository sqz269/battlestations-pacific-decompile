#include "bsp/native_mpak_record_copy.hpp"

#include "bsp/native_string_vector.hpp"
#include <cstring>

namespace bsp {
namespace {
using U = std::uint32_t;
static_assert(sizeof(void*) == 4);
void* at(const void* object, U offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<U>(object) + offset);
}
U word(const void* object, U offset = 0) noexcept {
    return *static_cast<const volatile U*>(at(object, offset));
}
void put(void* object, U offset, U value) noexcept {
    *static_cast<volatile U*>(at(object, offset)) = value;
}
void initialize_name(void* destination, const void* source,
    NativeStringStorage& strings) {
    put(destination, 0, 0);
    put(destination, 4, 0);
    if (destination == source) return;
    resize_native_string_header_0041dd40(destination, strings, word(source), true);
    if (word(source) != 0) {
        const U length = word(destination);
        void* const output = reinterpret_cast<void*>(word(destination, 4));
        const void* const input = reinterpret_cast<const void*>(word(source, 4));
        // Original BF7680 is named memcpy but includes backward-overlap copy.
        std::memmove(output, input, length);
    }
}
} // namespace

void* copy_construct_native_mpak_file_00bb65a0(void* destination,
    const void* source, NativeStringStorage& strings,
    NativeMpakOffsetVectorCopyLibrary& offsets) {
    initialize_name(destination, source, strings);
    put(destination, 8, word(source, 8));
    put(destination, 0xc, word(source, 0xc));
    const auto flag = *static_cast<const volatile std::uint8_t*>(at(source, 0x10));
    *static_cast<volatile std::uint8_t*>(at(destination, 0x10)) = flag;
    try {
        offsets.copy_00bb6180(at(destination, 0x14), at(source, 0x14));
    } catch (...) {
        // DFDE68 -> DFDE60 -> CC44F0 -> 41DD20: only the name is armed.
        destroy_native_string_header_0041dd20(destination, strings);
        throw;
    }
    return destination;
}

void* copy_construct_native_mpak_directory_00bb6630(void* destination,
    const void* source, NativeStringStorage& strings) {
    initialize_name(destination, source, strings);
    try {
        put(destination, 8, 0);
        put(destination, 0xc, 0);
        put(destination, 0x10, 0);
        copy_native_string_vector_00543e50(
            *static_cast<NativeStringVectorStorage*>(at(destination, 8)),
            *static_cast<const NativeStringVectorStorage*>(at(source, 8)), strings);
    } catch (...) {
        // DFDE94 -> DFDE8C -> CC4510 -> 41DD20: no member-vector rollback.
        destroy_native_string_header_0041dd20(destination, strings);
        throw;
    }
    return destination;
}

} // namespace bsp
