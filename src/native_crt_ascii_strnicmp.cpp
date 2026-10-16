#include "bsp/native_crt_ascii_strnicmp.hpp"

namespace bsp {
namespace {
using Word = std::uint32_t;
using Byte = unsigned char;
static_assert(sizeof(void*) == sizeof(Word));

Word read_word(Word address) {
    Word result;
    __asm {
        mov eax, address
        mov eax, dword ptr [eax]
        mov result, eax
    }
    return result;
}

Byte read_byte(Word address) {
    return *reinterpret_cast<const volatile Byte*>(address);
}

std::int32_t compare_bytes(Byte left, Byte right) {
    if (left == right) return 0;
    return left < right ? -1 : 1;
}
} // namespace

std::int32_t compare_native_crt_ascii_prefix_00c15700(
    const void* actual_parameters12) {
    const auto parameters = reinterpret_cast<Word>(actual_parameters12);
    Word remaining = read_word(parameters + 8u);
    if (remaining == 0) return 0;
    Word left = read_word(parameters);
    Word right = read_word(parameters + 4u);

    for (;;) {
        Byte left_byte = read_byte(left);
        Byte right_byte = read_byte(right);
        // Native MOV AL,[EDI] preserves the preceding OR AH,AH flags: the
        // right byte is read even when the left byte is already zero.
        if (left_byte == 0 || right_byte == 0) {
            return compare_bytes(left_byte, right_byte);
        }
        left += 1u;
        right += 1u;
        if (left_byte >= 0x41u && left_byte <= 0x5au) {
            left_byte = static_cast<Byte>(left_byte + 0x20u);
        }
        if (right_byte >= 0x41u && right_byte <= 0x5au) {
            right_byte = static_cast<Byte>(right_byte + 0x20u);
        }
        if (left_byte != right_byte) return compare_bytes(left_byte, right_byte);
        remaining -= 1u;
        if (remaining == 0) return 0;
    }
}

} // namespace bsp
