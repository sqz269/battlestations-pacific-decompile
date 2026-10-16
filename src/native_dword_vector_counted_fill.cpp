#include "bsp/native_dword_vector_counted_fill.hpp"

#include "bsp/native_dword_vector_copy_construct.hpp"
#include "bsp/native_word_vector_tidy.hpp"

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == sizeof(Word));
Word address(const void* value) { return reinterpret_cast<Word>(value); }
void* pointer(Word value) { return reinterpret_cast<void*>(value); }
} // namespace

__declspec(noinline) void __fastcall fill_native_dword_vector_counted_00698040(
    void* output, Word count, const void* source, Word unused_owner_bits,
    Word, Word) {
    const Word initial = address(output);
    Word current = initial;
    Word completed_end = initial;
    Word remaining = count;
    try {
        while (remaining != 0) {
            if (current != 0) {
                const void* current_source;
                __asm {
                    mov eax, source
                    mov current_source, eax
                }
                construct_native_checked_dword_vector_copy_00557590(
                    pointer(current), 0u, current_source);
            }
            --remaining;
            current += 16u;
            completed_end = current;
        }
    } catch (...) {
        Word cleanup_cursor = initial;
        const Word captured_completed_end = completed_end;
        if (cleanup_cursor != captured_completed_end) {
            Word captured_owner_bits;
            __asm {
                mov eax, unused_owner_bits
                mov captured_owner_bits, eax
            }
            do {
                tidy_native_word_vector_range_element_004d29d0(
                    pointer(captured_owner_bits), 0u, pointer(cleanup_cursor));
                cleanup_cursor += 16u;
            } while (cleanup_cursor != captured_completed_end);
        }
        throw;
    }
}

void* __stdcall fill_native_dword_vector_counted_dispatch_00698310(
    void* output, Word count, const void* source) {
    const Word captured_output = address(output);
    const Word captured_count = count;
    const Word source_bits = address(source);
    fill_native_dword_vector_counted_00698040(
        pointer(captured_output), captured_count, pointer(source_bits),
        0u, source_bits, 0u);
    return pointer(captured_output + (captured_count << 4u));
}

} // namespace bsp
