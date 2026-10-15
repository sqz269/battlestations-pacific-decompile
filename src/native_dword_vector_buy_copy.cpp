#include "bsp/native_dword_vector_buy_copy.hpp"

#include "bsp/native_dword_vector_allocation.hpp"

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4);

Word address(const void* value) noexcept {
    return reinterpret_cast<Word>(value);
}
void* pointer(Word value) noexcept {
    return reinterpret_cast<void*>(value);
}
Word read_word(Word location) noexcept {
    Word result;
    __asm {
        mov eax, location
        mov eax, dword ptr [eax]
        mov result, eax
    }
    return result;
}
void write_word(Word location, Word value) noexcept {
    __asm {
        mov eax, location
        mov edx, value
        mov dword ptr [eax], edx
    }
}
Word sar_two(Word value) noexcept {
    __asm {
        mov eax, value
        sar eax, 2
        mov value, eax
    }
    return value;
}
} // namespace

std::uint32_t __fastcall buy_native_checked_dword_vector_00557200(
    void* header, std::uint32_t, std::uint32_t count) {
    const Word requested = count;
    const Word receiver = address(header);
    write_word(receiver + 4u, 0u);
    write_word(receiver + 8u, 0u);
    write_word(receiver + 12u, 0u);
    if (requested == 0u)
        return 0u;
    if (requested > 0x3fffffffu)
        throw_native_dword_vector_length_error_00556c20();

    const Word begin = address(allocate_native_dword_vector_00553260(requested, nullptr));
    write_word(receiver + 4u, begin);
    write_word(receiver + 8u, begin);
    const Word capacity_end = begin + requested * 4u;
    write_word(receiver + 12u, capacity_end);
    return (capacity_end & 0xffffff00u) | 1u;
}

void* __cdecl copy_native_dword_range_forward_00697080(
    const void* first, const void* last, void* destination) {
    const Word captured_last = address(last);
    Word current = address(first);
    const Word output = address(destination);
    const Word result = output + sar_two(captured_last - current) * 4u;
    if (current != captured_last) {
        const Word displacement = output - current;
        do {
            const Word value = read_word(current);
            write_word(displacement + current, value);
            current += 4u;
        } while (current != captured_last);
    }
    return pointer(result);
}

} // namespace bsp
