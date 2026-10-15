#include "bsp/native_checked_bit_iterators.hpp"
#include "bsp/native_invalid_parameter_noinfo_call.hpp"

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == sizeof(Word));
Word address(const void* value) noexcept { return reinterpret_cast<Word>(value); }
void* pointer(Word value) noexcept { return reinterpret_cast<void*>(value); }
Word read(Word base, Word byte_offset) noexcept {
    Word value;
    __asm { mov eax, base }
    __asm { mov edx, byte_offset }
    __asm { mov eax, dword ptr [eax + edx] }
    __asm { mov value, eax }
    return value;
}
void write(Word base, Word byte_offset, Word value) noexcept {
    __asm { mov eax, base }
    __asm { mov edx, byte_offset }
    __asm { mov ecx, value }
    __asm { mov dword ptr [eax + edx], ecx }
}
void add_word(Word base, Word byte_offset, Word value) noexcept {
    __asm { mov eax, base }
    __asm { mov edx, byte_offset }
    __asm { mov ecx, value }
    __asm { add dword ptr [eax + edx], ecx }
}
Word sar_two(Word value) noexcept {
    Word result;
    __asm { mov eax, value }
    __asm { sar eax, 2 }
    __asm { mov result, eax }
    return result;
}
std::int32_t signed_word(Word value) noexcept {
    std::int32_t result;
    __asm { mov eax, value }
    __asm { mov result, eax }
    return result;
}
} // namespace

void* __fastcall advance_native_checked_bit_iterator_0048d3b0(void* iterator,
    Word, std::int32_t displacement) {
    if (displacement == 0) return iterator;
    const Word self = address(iterator);
    const Word distance = static_cast<Word>(displacement);
    const bool negative = displacement < 0;
    const Word magnitude = 0u - distance;
    if (read(self, 0u) == 0u || read(self, 4u) == 0u)
        invoke_native_invalid_parameter_00bf6713();
    const Word owner = read(self, 0u);
    const Word begin = read(owner, 8u);
    const Word captured_bit = read(self, 8u);
    if (negative) {
        if (begin > read(owner, 12u))
            invoke_native_invalid_parameter_00bf6713();
        const Word current_word = read(self, 4u);
        const Word position = (sar_two(current_word - begin) << 5u) + captured_bit;
        if (position < magnitude)
            invoke_native_invalid_parameter_00bf6713();
    } else {
        if (begin > read(owner, 12u))
            invoke_native_invalid_parameter_00bf6713();
        const Word current_word = read(self, 4u);
        const Word current_owner = read(self, 0u);
        const Word position = (sar_two(current_word - begin) << 5u)
            + captured_bit + distance;
        if (position > read(current_owner, 0u))
            invoke_native_invalid_parameter_00bf6713();
    }
    if (negative) {
        const Word current_bit = read(self, 8u);
        if (current_bit < magnitude) {
            const Word sum = current_bit + distance;
            const Word delta = 0xfffffffcu - (((0xffffffffu - sum) >> 5u) * 4u);
            add_word(self, 4u, delta);
            write(self, 8u, sum & 31u);
            return iterator;
        }
    }
    // Native common path reloads bit even after the negative borrow test.
    const Word sum = read(self, 8u) + distance;
    add_word(self, 4u, (sum >> 5u) * 4u);
    write(self, 8u, sum & 31u);
    return iterator;
}

void* __fastcall end_native_checked_bit_vector_0048f290(void* vector,
    Word, void* output) {
    const Word owner = address(vector);
    const Word result = address(output);
    const Word begin = read(owner, 8u);
    if (begin > read(owner, 12u)) invoke_native_invalid_parameter_00bf6713();
    write(result, 0u, 0u);
    write(result, 4u, begin);
    write(result, 8u, 0u);
    if (owner == 0u) invoke_native_invalid_parameter_00bf6713();
    write(result, 0u, owner);
    const Word bits = read(owner, 0u);
    if (bits != 0u)
        advance_native_checked_bit_iterator_0048d3b0(output, 0u, signed_word(bits));
    return output;
}

void* __fastcall copy_advance_native_checked_bit_iterator_0048f2e0(
    void* source, Word, void* output, std::int32_t displacement) {
    const Word input = address(source);
    const Word checked_owner = read(input, 0u);
    const Word captured_word = read(input, 4u);
    const Word captured_bit = read(input, 8u);
    Word private_iterator[3];
    private_iterator[1] = captured_word;
    private_iterator[2] = captured_bit;
    if (checked_owner == 0u) invoke_native_invalid_parameter_00bf6713();
    const Word current_owner = read(input, 0u);
    private_iterator[0] = current_owner;
    advance_native_checked_bit_iterator_0048d3b0(private_iterator,
        current_owner, displacement);
    const Word local = address(private_iterator);
    const Word owner = read(local, 0u);
    const Word word = read(local, 4u);
    const Word bit = read(local, 8u);
    const Word result = address(output);
    write(result, 0u, 0u);
    write(result, 4u, word);
    write(result, 8u, bit);
    if (owner == 0u) invoke_native_invalid_parameter_00bf6713();
    write(result, 0u, owner);
    return output;
}
} // namespace bsp
