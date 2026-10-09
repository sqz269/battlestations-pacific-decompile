#include "bsp/native_crt_free_unlock_helper.hpp"
#include "bsp/native_crt_canonical_unlock.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native CRT free unlock helper requires MSVC Win32.
#endif

namespace bsp {
static_assert(sizeof(void*) == 4);

// Preserve the separate helper identity and complete original instruction
// schedule, including ECX=4 rather than an equivalent stack ADD or tail jump.
__declspec(naked) void __cdecl unlock_native_crt_free_lock4_00bf9e1e() {
    __asm {
        push 4
        call unlock_native_crt_canonical_00c11b31
        pop ecx
        ret
    }
}

} // namespace bsp
