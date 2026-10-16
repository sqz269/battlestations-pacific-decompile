#include "bsp/native_crt_strrchr.hpp"

namespace bsp {
namespace {
using Byte = unsigned char;
using Uint32 = unsigned int;

static_assert(sizeof(void*) == 4);
static_assert(sizeof(int) == 4);
static_assert(sizeof(Uint32) == 4);
} // namespace

char* __cdecl find_native_crt_last_character_00bf87b0(
    char* actual_string, int character) {
    const volatile Byte* const bytes =
        reinterpret_cast<const volatile Byte*>(actual_string);

    Uint32 nul_offset = 0;
    while (bytes[nul_offset] != 0U) {
        ++nul_offset;
    }

    const Byte target = static_cast<Byte>(character);
    Uint32 candidate_offset = nul_offset;
    Uint32 remaining = nul_offset + 1U;
    for (;;) {
        const Byte current = bytes[candidate_offset];
        --remaining;
        if (current == target || remaining == 0U) {
            break;
        }
        --candidate_offset;
    }

    const Byte final_byte = bytes[candidate_offset];
    if (final_byte != target) {
        return nullptr;
    }
    return actual_string + candidate_offset;
}

} // namespace bsp
