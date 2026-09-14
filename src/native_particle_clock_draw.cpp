#include "bsp/native_particle_clock_draw.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native particle-clock draw requires MSVC Win32.
#endif

namespace bsp {
static_assert(sizeof(void*) == 4 && sizeof(float) == 4);

__declspec(naked) void __fastcall set_native_particle_clock_time_00b19a10(
    void*, void*, float) {
    __asm {
        movss xmm0, dword ptr [esp + 4]
        push esi
        push edi
        mov edi, ecx
        mov esi, dword ptr [edi + 8]
        movss dword ptr [edi + 18h], xmm0
        mov eax, dword ptr [edi + 0Ch]
        imul eax, eax, 2Ch
        add eax, esi
        cmp esi, eax
        je complete
        mov edi, edi
    next_record:
        mov ecx, dword ptr [esi + 28h]
        fld dword ptr [esp + 0Ch]
        mov edx, dword ptr [ecx]
        mov eax, dword ptr [edx + 18h]
        push ecx
        fstp dword ptr [esp]
        call eax
        mov ecx, dword ptr [edi + 0Ch]
        imul ecx, ecx, 2Ch
        add ecx, dword ptr [edi + 8]
        add esi, 2Ch
        cmp esi, ecx
        jne next_record
    complete:
        pop edi
        pop esi
        ret 4
    }
}
} // namespace bsp
