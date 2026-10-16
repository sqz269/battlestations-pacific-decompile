#include "bsp/native_crt_v6_unlock_helper.hpp"
#include "bsp/native_crt_canonical_unlock.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native CRT V6 unlock helper requires MSVC Win32.
#endif

namespace bsp {
static_assert(sizeof(void*) == 4);

// Preserve the separate actual child call and ECX=4 from POP. Do not alias
// the calloc helper, tail-jump, use a stack ADD or model a private SEH frame.
__declspec(naked) void __cdecl unlock_native_crt_v6_heap_00bf9e9c() {
    __asm {
        push 4
        call unlock_native_crt_canonical_00c11b31
        pop ecx
        ret
    }
}

} // namespace bsp
