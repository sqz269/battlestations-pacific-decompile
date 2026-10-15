#include "bsp/native_dword_inplace_ranges.hpp"

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == sizeof(Word));
Word address(const void* value) noexcept { return reinterpret_cast<Word>(value); }
void* pointer(Word value) noexcept { return reinterpret_cast<void*>(value); }
Word read_word(Word location) noexcept {
    Word value;
    __asm { mov eax, location }
    __asm { mov eax, dword ptr [eax] }
    __asm { mov value, eax }
    return value;
}
void write_word(Word location, Word value) noexcept {
    __asm { mov eax, location }
    __asm { mov ecx, value }
    __asm { mov dword ptr [eax], ecx }
}
Word sar_two(Word value) noexcept {
    Word result;
    __asm { mov eax, value }
    __asm { sar eax, 2 }
    __asm { mov result, eax }
    return result;
}
} // namespace

void* __cdecl fill_native_dword_inplace_range_006970b0(void* first,
    const void* last, const void* value) {
    Word cursor = address(first);
    const Word captured_last = address(last);
    if (cursor != captured_last) {
        const Word captured_value_address = address(value);
        do {
            const Word current_value = read_word(captured_value_address);
            write_word(cursor, current_value);
            cursor += 4u;
        } while (cursor != captured_last);
    }
    return pointer(captured_last);
}

void* __cdecl copy_backward_native_dword_range_00697440(const void* first,
    const void* last, void* destination_end) {
    const Word captured_first = address(first);
    Word cursor = address(last);
    const Word captured_destination_end = address(destination_end);
    const Word count = sar_two(cursor - captured_first);
    const Word returned = captured_destination_end - (count * 4u);
    if (cursor != captured_first) {
        const Word displacement = captured_destination_end - cursor;
        do {
            const Word current_value = read_word(cursor - 4u);
            cursor -= 4u;
            const bool more = cursor != captured_first;
            write_word(displacement + cursor, current_value);
            if (!more) break;
        } while (true);
    }
    return pointer(returned);
}
} // namespace bsp
