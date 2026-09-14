#include "bsp/native_material_diagnostics_record_init.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native material diagnostics record initialization requires MSVC Win32.
#endif

namespace bsp {
__declspec(naked) void* __fastcall initialize_native_material_diagnostics_record_00b106c0(void*) {
    __asm {
        mov eax, ecx
        push esi
        xor esi, esi
        mov dword ptr [eax + 011ch], esi
        mov dword ptr [eax + 0120h], esi
        mov dword ptr [eax + 0124h], esi
        mov dword ptr [eax + 0128h], esi
        mov dword ptr [eax], esi
        lea edx, [eax + 03ch]
        lea ecx, [esi + 0eh]
    clear_mode:
        mov dword ptr [edx - 038h], esi
        mov dword ptr [edx], esi
        mov dword ptr [edx + 038h], esi
        mov dword ptr [edx + 070h], esi
        mov dword ptr [edx + 0a8h], esi
        add edx, 4
        sub ecx, 1
        jnz clear_mode
        pop esi
        ret
    }
}
} // namespace bsp
