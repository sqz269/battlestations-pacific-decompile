#include "bsp/native_crt_wcschr.hpp"

namespace bsp {
namespace {
using Byte = unsigned char;
using Uint32 = unsigned int;

static_assert(sizeof(void*) == 4);
static_assert(sizeof(int) == 4);
static_assert(sizeof(Uint32) == 4);

// Addressed WORD16 read over possibly unaligned genuine byte storage, without
// inventing a typed WORD object. MOVZX initializes all 32 result bits. Verify
// the actual strict/Main objects for load width, ordering and NUL-only reread.
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

void* __cdecl find_native_crt_wide_character_00bfd6c1(
    void* actual_string, int character) {
    Byte* current = static_cast<Byte*>(actual_string);
    const Uint32 target = static_cast<Uint32>(character) & 0xffffU;
    for (;;) {
        const Uint32 current_word = read_unsigned_word(current);
        if (current_word == 0U) {
            const Uint32 terminal_word = read_unsigned_word(current);
            if (terminal_word != target) {
                return nullptr;
            }
            return current;
        }
        if (current_word == target) {
            return current;
        }
        current += 2U;
    }
}

} // namespace bsp
