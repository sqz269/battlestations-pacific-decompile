#include "bsp/native_crt_calloc_unlock4_helper.hpp"
#include "bsp/native_crt_canonical_unlock.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native CRT calloc lock-4 unlock helper requires MSVC Win32.
#endif

namespace bsp {
static_assert(sizeof(void*) == 4);

// Keep the separate actual child call and ECX=4 from POP, not a tail jump,
// stack ADD, C++ wrapper or emulation of the enclosing private-frame action.
__declspec(naked) void __cdecl unlock_native_crt_calloc_lock4_00c1c60b() {
    __asm {
        push 4
        call unlock_native_crt_canonical_00c11b31
        pop ecx
        ret
    }
}

} // namespace bsp
