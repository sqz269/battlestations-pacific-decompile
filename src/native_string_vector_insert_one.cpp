#include "bsp/native_string_vector_insert_one.hpp"

#include "bsp/native_invalid_parameter_noinfo_call.hpp"
#include "bsp/native_string_vector_insert.hpp"

#include <cstring>
#include <intrin.h>

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == sizeof(Word));

Word address(const void* value) noexcept {
    return reinterpret_cast<Word>(value);
}
Word word(Word base, Word offset) noexcept {
    Word value;
    _ReadWriteBarrier();
    std::memcpy(&value, reinterpret_cast<const void*>(base + offset), sizeof value);
    _ReadWriteBarrier();
    return value;
}
void put(Word base, Word offset, Word value) noexcept {
    _ReadWriteBarrier();
    std::memcpy(reinterpret_cast<void*>(base + offset), &value, sizeof value);
    _ReadWriteBarrier();
}
Word sar_three(Word value) noexcept {
    return (value >> 3) | ((value & 0x80000000u) ? 0xe0000000u : 0u);
}
} // namespace

void* __fastcall insert_native_string_header_vector_one_00450180(void* header,
    Word, void* output, const void* owner, void* position, const void* value,
    NativeStringRawPoolContext& strings) {
    const Word header_address = address(header);
    const Word original_begin = word(header_address, 4);
    Word offset = 0u;
    if (original_begin != 0u) {
        const Word original_end = word(header_address, 8);
        if (sar_three(original_end - original_begin) != 0u) {
            if (original_begin > original_end)
                invoke_native_invalid_parameter_00bf6713();
            if (owner == nullptr || owner != header)
                invoke_native_invalid_parameter_00bf6713();
            offset = sar_three(address(position) - original_begin);
        }
    }

    //004501C1 overwrites incoming EDX with the stacked value-header address.
    insert_native_string_header_vector_count_0044f410(header, address(value),
        owner, position, 1u, value, strings);

    const Word new_begin = word(header_address, 4);
    if (new_begin > word(header_address, 8))
        invoke_native_invalid_parameter_00bf6713();
    // Retain new_begin even if the returning handler changes the header.
    const Word inserted = new_begin + offset * 8u;
    if (inserted > word(header_address, 8) ||
        inserted < word(header_address, 4))
        invoke_native_invalid_parameter_00bf6713();

    const Word output_address = address(output);
    put(output_address, 4, inserted);
    put(output_address, 0, header_address);
    return output;
}
} // namespace bsp
