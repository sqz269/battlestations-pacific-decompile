#include "bsp/native_string_vector_casefold_equal.hpp"

#include <cstdint>
#include <cstring>

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == sizeof(Word));
Word word(Word address) noexcept {
    Word value;
    std::memcpy(&value, reinterpret_cast<const void*>(address), sizeof(value));
    return value;
}
bool positive(Word value) noexcept { return value != 0u && value < 0x80000000u; }
// Signed comparison without implementation-defined unsigned-to-signed casts.
bool signed_less(Word left, Word right) noexcept {
    return (left ^ 0x80000000u) < (right ^ 0x80000000u);
}
} // namespace

bool equal_native_string_vector_casefold_008d48e0(
    const void* vector_header, const void* other_header) {
    const Word left = reinterpret_cast<Word>(vector_header);
    const Word right = reinterpret_cast<Word>(other_header);
    const Word initial_count = word(right + 4u);
    if (word(left + 4u) != initial_count) return false;
    if (!positive(initial_count)) return true;
    Word index = 0u;
    do {
        const Word left_element = word(left) + index * 8u;
        const Word right_element = word(right) + index * 8u;
        if (word(left_element) == 0u) {
            if (word(right_element) != 0u) return false;
        } else {
            if (word(right_element) == 0u) return false;
            const Word right_text = word(right_element + 4u);
            const Word left_text = word(left_element + 4u);
            if (_stricmp(reinterpret_cast<const char*>(left_text),
                    reinterpret_cast<const char*>(right_text)) != 0)
                return false;
        }
        index += 1u;
    } while (signed_less(index, word(right + 4u)));
    return true;
}
} // namespace bsp
