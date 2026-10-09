#include "bsp/native_crt_lock10_unlock_helper.hpp"
#include "bsp/native_crt_canonical_unlock.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native CRT lock-10 unlock helper requires MSVC Win32.
#endif

namespace bsp {
static_assert(sizeof(void*) == 4);

// Retain the complete helper identity and ECX=10 from POP, including the
// separate child call rather than a tail jump or an equivalent stack ADD.
__declspec(naked) void __cdecl unlock_native_crt_initializer_lock10_00c11c18() {
    __asm {
        push 10
        call unlock_native_crt_canonical_00c11b31
        pop ecx
        ret
    }
}

} // namespace bsp
