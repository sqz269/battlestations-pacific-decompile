#include "bsp/native_crt_wcscspn.hpp"

namespace bsp {
namespace {
using Byte = unsigned char;
using Uint32 = unsigned int;

static_assert(sizeof(void*) == 4);
static_assert(sizeof(Uint32) == 4);

// Read one WORD over genuine possibly unaligned raw byte storage. MOVZX writes
// all 32 result bits, without a typed wchar_t/uint16_t object dereference. Each
// separately sequenced invocation must remain one ordered WORD payload read in
// the actual strict/Main object, including every emitted private helper variant.
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

unsigned int __cdecl count_native_crt_wide_prefix_excluding_00c0382c(
    const void* actual_source, const void* actual_control) {
    const Byte* source = static_cast<const Byte*>(actual_source);
    const Uint32 first_source_guard = read_unsigned_word(source);
    if (first_source_guard == 0U) {
        return 0U;
    }

    const Byte* const control_base = static_cast<const Byte*>(actual_control);
    const Uint32 first_control_word = read_unsigned_word(control_base);
    Uint32 traversed = 0U;
    for (;;) {
        const Byte* control = control_base;
        if (first_control_word != 0U) {
            const Uint32 source_value = read_unsigned_word(source);
            Uint32 control_value = first_control_word;
            for (;;) {
                if (control_value == source_value) {
                    return traversed | ((traversed & 0x40000000U) << 1);
                }
                ++control;
                ++control;
                control_value = read_unsigned_word(control);
                if (control_value == 0U) {
                    break;
                }
            }
        }

        ++source;
        ++source;
        ++traversed;
        const Uint32 next_source_guard = read_unsigned_word(source);
        if (next_source_guard == 0U) {
            return traversed | ((traversed & 0x40000000U) << 1);
        }
    }
}

} // namespace bsp
