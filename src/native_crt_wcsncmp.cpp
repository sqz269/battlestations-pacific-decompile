#include "bsp/native_crt_wcsncmp.hpp"

namespace bsp {
namespace {
using Byte = unsigned char;
using Uint32 = unsigned int;

static_assert(sizeof(void*) == 4);
static_assert(sizeof(int) == 4);
static_assert(sizeof(Uint32) == 4);

// Read exactly one possibly unaligned WORD from genuine raw byte storage.
// MOVZX initializes all 32 result bits without inventing a typed WORD object.
// The actual compiler object must preserve the separate ordered load sites.
__forceinline Uint32 read_unsigned_word(const Byte* address) {
    Uint32 value;
    __asm {
        mov eax, address
        movzx eax, word ptr [eax]
        mov value, eax
    }
    return value;
}
} // namespace

int __cdecl compare_native_crt_wide_prefix_00bfdcb7(
    const void* actual_left, const void* actual_right, unsigned int count) {
    if (count == 0U) {
        return 0;
    }

    const Byte* left = static_cast<const Byte*>(actual_left);
    const Byte* right = static_cast<const Byte*>(actual_right);
    Uint32 remaining = count;
    for (;;) {
        --remaining;
        if (remaining == 0U) {
            break;
        }
        const Uint32 preliminary_left = read_unsigned_word(left);
        if (preliminary_left == 0U) {
            break;
        }
        const Uint32 preliminary_right = read_unsigned_word(right);
        if (preliminary_left != preliminary_right) {
            break;
        }
        left += 2U;
        right += 2U;
    }

    const Uint32 final_left = read_unsigned_word(left);
    const Uint32 final_right = read_unsigned_word(right);
    return static_cast<int>(final_left) - static_cast<int>(final_right);
}

} // namespace bsp
