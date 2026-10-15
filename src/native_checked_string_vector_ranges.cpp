#include "bsp/native_checked_string_vector_ranges.hpp"

#include "bsp/native_string_vector_assignment.hpp"

#include <cstdint>

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == sizeof(Word));
Word bits(const void* value) noexcept { return reinterpret_cast<Word>(value); }
void* pointer(Word value) noexcept { return reinterpret_cast<void*>(value); }
Word sar_four(Word value) noexcept {
    return (value >> 4) | ((value & 0x80000000u) ? 0xf0000000u : 0u);
}
} // namespace

void* __cdecl copy_native_checked_string_vector_range_006a29c0(
    const void* first, const void* last, void* destination,
    NativeStringRawPoolContext& strings) {
    Word cursor = bits(first);
    const Word finish = bits(last);
    const Word destination_begin = bits(destination);
    // 006A29D0..DA computes this before the first assignment, independently of
    // child returns. Retain wrapped SUB/SAR4/SHL4/ADD, including high-bit deltas.
    const Word completed_end = destination_begin + (sar_four(finish - cursor) << 4);
    if (cursor != finish) {
        const Word destination_delta = destination_begin - cursor; //006A29E0.
        do {
            assign_native_string_header_vector_00506c80(
                pointer(cursor + destination_delta), pointer(cursor), strings);
            cursor += 0x10u;
        } while (cursor != finish);
    }
    return pointer(completed_end);
}

void __cdecl fill_native_checked_string_vector_range_006a2a00(
    void* first, const void* last, const void* value,
    NativeStringRawPoolContext& strings) {
    Word cursor = bits(first);
    const Word finish = bits(last);
    if (cursor != finish) {
        const void* const captured_value = value; // Pointer only; no header copy.
        do {
            assign_native_string_header_vector_00506c80(
                pointer(cursor), captured_value, strings);
            cursor += 0x10u;
        } while (cursor != finish);
    }
}
} // namespace bsp
