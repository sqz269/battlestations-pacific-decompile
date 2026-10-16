#include "bsp/native_crt_strcspn.hpp"

static_assert(sizeof(void*) == 4, "The addressed Original uses 32-bit pointers.");
static_assert(sizeof(unsigned int) == 4, "The bitmap and result use unsigned32.");

namespace bsp {

unsigned int __cdecl count_native_crt_prefix_excluding_00bf9a50(
    const void* actual_source, const void* actual_reject)
{
    // Separate scalar objects avoid introducing an array/aggregate GS buffer.
    // Volatile private words retain explicit initialization without a provider.
    volatile unsigned int word0 = 0U;
    volatile unsigned int word1 = 0U;
    volatile unsigned int word2 = 0U;
    volatile unsigned int word3 = 0U;
    volatile unsigned int word4 = 0U;
    volatile unsigned int word5 = 0U;
    volatile unsigned int word6 = 0U;
    volatile unsigned int word7 = 0U;

    const auto* reject_cursor =
        static_cast<const volatile unsigned char*>(actual_reject);
    for (;;) {
        const unsigned int byte = *reject_cursor;
        if (byte == 0U) {
            break;
        }
        ++reject_cursor;

        const unsigned int word_index = byte >> 5U;
        const unsigned int mask = 1U << (byte & 31U);
        volatile unsigned int* selected_word;
        if (word_index < 4U) {
            if (word_index < 2U) {
                selected_word = word_index == 0U ? &word0 : &word1;
            } else {
                selected_word = word_index == 2U ? &word2 : &word3;
            }
        } else {
            if (word_index < 6U) {
                selected_word = word_index == 4U ? &word4 : &word5;
            } else {
                selected_word = word_index == 6U ? &word6 : &word7;
            }
        }
        const unsigned int previous = *selected_word;
        *selected_word = previous | mask;
    }

    const auto* source_cursor =
        static_cast<const volatile unsigned char*>(actual_source);
    unsigned int prefix_count = 0U;
    for (;;) {
        const unsigned int byte = *source_cursor;
        if (byte == 0U) {
            return prefix_count;
        }
        ++source_cursor;

        const unsigned int word_index = byte >> 5U;
        const unsigned int mask = 1U << (byte & 31U);
        volatile unsigned int* selected_word;
        if (word_index < 4U) {
            if (word_index < 2U) {
                selected_word = word_index == 0U ? &word0 : &word1;
            } else {
                selected_word = word_index == 2U ? &word2 : &word3;
            }
        } else {
            if (word_index < 6U) {
                selected_word = word_index == 4U ? &word4 : &word5;
            } else {
                selected_word = word_index == 6U ? &word6 : &word7;
            }
        }
        const unsigned int membership = *selected_word;
        if ((membership & mask) != 0U) {
            return prefix_count;
        }
        ++prefix_count;
    }
}

} // namespace bsp
