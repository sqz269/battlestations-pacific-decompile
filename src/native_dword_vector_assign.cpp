#include "bsp/native_dword_vector_assign.hpp"

#include "bsp/native_dword_assignment_copy_dispatch.hpp"
#include "bsp/native_dword_vector_buy_copy.hpp"
#include "bsp/native_dword_vector_erase.hpp"
#include "bsp/singleton_lifetime.hpp"

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

void* __fastcall assign_native_checked_dword_vector_00697bd0(
    void* destination, std::uint32_t, const void* source) {
    const Word receiver = address(destination);
    const Word input = address(source);
    if (receiver == input)
        return destination;

    const Word source_begin = read_word(input + 4u);
    if (source_begin == 0u) {
        clear_native_checked_dword_vector_00697890(destination, 0u);
        return destination;
    }
    const Word source_end = read_word(input + 8u);
    const Word source_size = sar_two(source_end - source_begin);
    if (source_size == 0u) {
        clear_native_checked_dword_vector_00697890(destination, 0u);
        return destination;
    }

    const Word destination_begin = read_word(receiver + 4u);
    Word destination_size = 0u;
    if (destination_begin != 0u)
        destination_size = sar_two(read_word(receiver + 8u) - destination_begin);

    if (source_size <= destination_size) {
        copy_native_dword_range_forward_00697080(
            pointer(source_begin), pointer(source_end), pointer(destination_begin));
        const Word current_source_begin = read_word(input + 4u);
        Word source_delta = 0u;
        if (current_source_begin != 0u)
            source_delta = read_word(input + 8u) - current_source_begin;
        const Word current_destination_begin = read_word(receiver + 4u);
        const Word current_source_size = current_source_begin == 0u
            ? 0u : sar_two(source_delta);
        write_word(receiver + 8u,
            current_destination_begin + current_source_size * 4u);
        return destination;
    }

    Word capacity = 0u;
    if (destination_begin != 0u)
        capacity = sar_two(read_word(receiver + 12u) - destination_begin);
    if (source_size <= capacity) {
        Word current_size = 0u;
        if (destination_begin != 0u)
            current_size = sar_two(read_word(receiver + 8u) - destination_begin);
        const Word source_middle = source_begin + current_size * 4u;
        copy_native_dword_range_forward_00697080(
            pointer(source_begin), pointer(source_middle), pointer(destination_begin));
        const Word tail_destination = read_word(receiver + 8u);
        const Word current_source_end = read_word(input + 8u);
        const Word copied_end = address(
            copy_native_dword_assignment_uninitialized_dispatch_006974b0(
                destination, tail_destination, pointer(source_middle),
                pointer(current_source_end), pointer(tail_destination)));
        write_word(receiver + 8u, copied_end);
        return destination;
    }

    if (destination_begin != 0u)
        singleton_lifetime_free(pointer(destination_begin));

    const Word current_source_begin = read_word(input + 4u);
    Word requested = 0u;
    if (current_source_begin != 0u)
        requested = sar_two(read_word(input + 8u) - current_source_begin);
    const Word bought = buy_native_checked_dword_vector_00557200(
        destination, 0u, requested);
    if ((bought & 0xffu) != 0u) {
        const Word new_destination_begin = read_word(receiver + 4u);
        const Word new_source_end = read_word(input + 8u);
        const Word new_source_begin = read_word(input + 4u);
        const Word copied_end = address(
            copy_native_dword_assignment_uninitialized_dispatch_006974b0(
                destination, new_source_end, pointer(new_source_begin),
                pointer(new_source_end), pointer(new_destination_begin)));
        write_word(receiver + 8u, copied_end);
    }
    return destination;
}

} // namespace bsp
