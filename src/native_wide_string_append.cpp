#include "bsp/native_wide_string_append.hpp"
#include "bsp/native_wide_string.hpp"

#include <cstdint>
#include <cstring>
#include <intrin.h>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw wide append requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4 && sizeof(std::uintptr_t) == 4 && sizeof(Word) == 4,
    "requires the Win32 address model");

Word word(const void* header, Word offset) noexcept {
    const auto address = reinterpret_cast<std::uintptr_t>(header) + offset;
    Word value;
    _ReadWriteBarrier();
    std::memcpy(&value, reinterpret_cast<const void*>(address), sizeof value);
    _ReadWriteBarrier();
    return value;
}
void* pointer(Word bits) noexcept {
    return reinterpret_cast<void*>(static_cast<std::uintptr_t>(bits));
}
} // namespace

void append_native_wide_string_header_005cf090(void* destination,
    const void* source, NativeStringRawPoolContext& strings) {
    const Word source_length = word(source, 0u); // EDI, before any dst read
    if (source_length == 0u) return;
    const Word old_destination_length = word(destination, 0u); // EBX
    const Word length = source_length + old_destination_length;
    resize_native_wide_string_header_004c53e0(destination, strings, length, true);
    const Word source_data = word(source, 4u); // CURRENT source FIRST, 005CF0AE
    const Word destination_data = word(destination, 4u); //005CF0B1
    const Word bytes = source_length * 2u;
    const Word output = destination_data + old_destination_length * 2u;
    // Preserve both operand reads even for nonzero length whose bytes wrap0.
    // BF7680/current memmove overlap is qualified; zero-byte call is omitted.
    if (bytes != 0u) std::memmove(pointer(output), pointer(source_data), bytes);
}
} // namespace bsp
