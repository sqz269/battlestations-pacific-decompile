#include "bsp/native_crt_strchr.hpp"

#include <cstdint>

namespace bsp {
namespace {
using Word = std::uint32_t;
using Byte = unsigned char;
static_assert(sizeof(void*) == sizeof(Word));
Word address(const void* value) noexcept { return reinterpret_cast<Word>(value); }
void* pointer(Word value) noexcept { return reinterpret_cast<void*>(value); }
Word read_word(Word value) noexcept {
    return *reinterpret_cast<const volatile Word*>(value);
}
Byte read_byte(Word value) noexcept {
    return *reinterpret_cast<const volatile Byte*>(value);
}
} // namespace

void* find_native_crt_character_00bf86f0(const void* actual_parameters8) {
    const Word parameters = address(actual_parameters8);
    const Byte sought = read_byte(parameters + 4u); // BF86F2, BEFORE pointer.
    Word cursor = read_word(parameters);          // BF86FC.

    while ((cursor & 3u) != 0) {
        const Byte byte = read_byte(cursor); // Original alignment path is BYTE.
        cursor += 1u;
        if (byte == sought) return pointer(cursor - 1u); // Pre-entry BF86E0 tail.
        if (byte == 0) return nullptr;
    }

    Word repeated = static_cast<Word>(sought);
    repeated |= repeated << 8u;
    repeated |= repeated << 16u;
    for (;;) {
        const Word current = read_word(cursor); // BF8728, aligned DWORD.
        const Word different = current ^ repeated;
        const Word original_sum = current + 0x7efefeffu;
        const Word different_sum = different + 0x7efefeffu;
        const Word candidate = (~different ^ different_sum) & 0x81010100u;
        cursor += 4u;
        if (candidate != 0) {
            const Word bytes = read_word(cursor - 4u); // BF876A, actual reread.
            for (Word index = 0; index != 4u; ++index) {
                const Byte byte = static_cast<Byte>(bytes >> (index * 8u));
                if (byte == sought) return pointer(cursor - 4u + index);
                if (byte == 0) return nullptr;
            }
            continue;
        }

        const Word zero_candidate = (~current ^ original_sum) & 0x81010100u;
        if (zero_candidate == 0) continue;
        if ((zero_candidate & 0x01010100u) != 0) return nullptr;
        if ((original_sum & 0x80000000u) == 0) return nullptr;
    }
}

} // namespace bsp
