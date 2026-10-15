#include "bsp/native_dword_fill_ranges.hpp"

#include <cstring>

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4 && sizeof(Word) == 4);

Word address(const void* value) noexcept {
    return reinterpret_cast<Word>(value);
}
void* pointer(Word value) noexcept {
    return reinterpret_cast<void*>(value);
}
Word load_word(Word source) noexcept {
    Word value;
    std::memcpy(&value, pointer(source), sizeof(value));
    return value;
}
void store_word(Word destination, Word value) noexcept {
    std::memcpy(pointer(destination), &value, sizeof(value));
}
} // namespace

void* __cdecl fill_native_dword_range_0043f9e0(void* first,
    const void* last, const void* value) {
    Word cursor = address(first);
    const Word captured_last = address(last);
    if (cursor != captured_last) {
        const Word source = address(value);
        do {
            const Word current_value = load_word(source);
            store_word(cursor, current_value);
            cursor += 4u;
        } while (cursor != captured_last);
    }
    return pointer(captured_last);
}

void* __stdcall fill_native_dword_count_00441180(void* first,
    std::uint32_t count, const void* value) {
    const Word captured_count = count;
    const Word captured_first = address(first);
    Word remaining = captured_count;
    Word cursor = captured_first;
    if (captured_count != 0u) {
        const Word source = address(value);
        do {
            const Word current_value = load_word(source);
            store_word(cursor, current_value);
            --remaining;
            cursor += 4u;
        } while (remaining != 0u);
    }
    return pointer(captured_first + (captured_count << 2u));
}
} // namespace bsp
