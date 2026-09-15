#include "bsp/native_shadow_pointer_vector_disposal.hpp"
#include "bsp/native_shadow_pointer_vector.hpp"
#include "bsp/singleton_lifetime.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native shadow pointer-vector disposal requires MSVC Win32 raw assembly.
#endif

namespace bsp {
static_assert(sizeof(void*) == 4);

__declspec(naked) void __fastcall destroy_native_shadow_pointer_vector_00ae1c60(
    void*, void*) {
    __asm {
        push esi // AE1C60
        push 0 // AE1C61
        mov esi, ecx // AE1C63; actual array header, not enclosing entry
        call resize_native_shadow_pointer_vector_00ae19b0 // AE1C65; full body, RET4
        mov eax, dword ptr [esi] // AE1C6A; CURRENT data after resize
        push eax // AE1C6C; unconditional, including null
        call singleton_lifetime_free // AE1C6D -> actual BF6989 source CRT domain
        add esp, 4 // AE1C72
        pop esi // AE1C75
        _emit 0c3 // AE1C76; preserve exact single-byte RET
    }
}
} // namespace bsp
