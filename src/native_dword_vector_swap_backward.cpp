#include "bsp/native_dword_vector_swap_backward.hpp"

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == sizeof(Word));

Word address(const void* value) { return reinterpret_cast<Word>(value); }
void* pointer(Word value) { return reinterpret_cast<void*>(value); }

Word read_word(Word location) {
    Word value;
    __asm { mov eax, location }
    __asm { mov eax, dword ptr [eax] }
    __asm { mov value, eax }
    return value;
}

void write_word(Word location, Word value) {
    __asm { mov eax, location }
    __asm { mov ecx, value }
    __asm { mov dword ptr [eax], ecx }
}

Word sar_four(Word value) {
    Word result;
    __asm { mov eax, value }
    __asm { sar eax, 4 }
    __asm { mov result, eax }
    return result;
}
} // namespace

__declspec(noinline) void* __cdecl
swap_native_dword_vector_headers_backward_006981c0(
    void* first, void* last, void* output_end, Word, Word, Word) {
    Word source_cursor;
    Word output_cursor;
    Word captured_first;
    // Keep these three private captures before even the empty-range test.
    __asm {
        mov eax, last
        mov source_cursor, eax
        mov eax, output_end
        mov output_cursor, eax
        mov eax, first
        mov captured_first, eax
    }
    if (source_cursor != captured_first) {
        do {
            const Word source_begin = read_word(source_cursor - 12u);
            const Word output_begin = read_word(output_cursor - 12u);
            source_cursor -= 16u;
            output_cursor -= 16u;
            const bool more = source_cursor != captured_first;
            write_word(output_cursor + 4u, source_begin);
            write_word(source_cursor + 4u, output_begin);

            const Word source_end = read_word(source_cursor + 8u);
            const Word output_end_word = read_word(output_cursor + 8u);
            write_word(output_cursor + 8u, source_end);
            write_word(source_cursor + 8u, output_end_word);

            const Word source_capacity = read_word(source_cursor + 12u);
            const Word output_capacity = read_word(output_cursor + 12u);
            write_word(output_cursor + 12u, source_capacity);
            write_word(source_cursor + 12u, output_capacity);
            if (!more) break;
        } while (true);
    }
    return pointer(output_cursor);
}

void* __cdecl swap_native_dword_vector_headers_backward_dispatch_00698380(
    void* first, void* last, void* output_end) {
    const Word captured_first = address(first);
    const Word captured_last = address(last);
    const Word captured_output = address(output_end);
    const Word canonical_private_tag = 0;
    swap_native_dword_vector_headers_backward_006981c0(
        pointer(captured_first), pointer(captured_last), pointer(captured_output),
        captured_output, captured_output, canonical_private_tag);
    const Word rounded_distance = sar_four(captured_last - captured_first) << 4u;
    return pointer(captured_output - rounded_distance);
}

} // namespace bsp
