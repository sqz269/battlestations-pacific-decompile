#include "bsp/native_crt_strstr.hpp"
#include "bsp/native_crt_strchr.hpp"

namespace bsp {
namespace {
using Byte = unsigned char;
using Word = unsigned int;
static_assert(sizeof(Byte) == 1 && sizeof(Word) == 4 && sizeof(void*) == 4);

// Real objects, not Original argument homes or a register-entry adapter.
struct alignas(4) CharacterParameters {
    Word haystack;
    Word sought;
};
static_assert(sizeof(CharacterParameters) == 8);
static_assert(alignof(CharacterParameters) == 4);
} // namespace

void* __cdecl find_native_crt_substring_00bf9440(
    const void* actual_haystack, const void* actual_needle) {
    const auto* needle = static_cast<const Byte*>(actual_needle);
    const Byte first = *static_cast<const volatile Byte*>(actual_needle);
    if (first == 0) return const_cast<void*>(actual_haystack);

    const Byte second = *static_cast<const volatile Byte*>(needle + 1);
    if (second == 0) {
        const CharacterParameters parameters{
            reinterpret_cast<Word>(actual_haystack), static_cast<Word>(first)};
        return find_native_crt_character_00bf86f0(&parameters);
    }

    const auto* restart = static_cast<const Byte*>(actual_haystack);
    for (;;) {
        const Byte* cursor = restart;
        const Byte* current_address = cursor;
        Byte current = *static_cast<const volatile Byte*>(cursor);
        ++cursor;

        for (;;) {
            if (current != first) {
                if (current == 0) return nullptr;
                current_address = cursor;
                current = *static_cast<const volatile Byte*>(cursor);
                ++cursor;
                continue;
            }

            const Byte* const candidate = current_address;
            const Byte* const next_attempt = cursor; // Historical q + 1.
            current_address = cursor;
            current = *static_cast<const volatile Byte*>(cursor);
            ++cursor;
            if (current != second) continue; // Retry this captured BYTE.

            const Byte* needle_cursor = needle;
            for (;;) {
                const Byte* const first_extension = needle_cursor + 2;
                const Byte expected =
                    *static_cast<const volatile Byte*>(first_extension);
                if (expected == 0) return const_cast<Byte*>(candidate);

                const Byte actual = *static_cast<const volatile Byte*>(cursor);
                cursor += 2; // Required even on mismatch or next-needle NUL.
                if (actual != expected) break;

                const Byte expected_partner =
                    *static_cast<const volatile Byte*>(first_extension + 1);
                if (expected_partner == 0) return const_cast<Byte*>(candidate);

                const Byte actual_partner =
                    *static_cast<const volatile Byte*>(cursor - 1);
                needle_cursor += 2; // After partner read, before comparison.
                if (actual_partner != expected_partner) break;
            }
            restart = next_attempt;
            break;
        }
    }
}

} // namespace bsp
