#include "bsp/native_texture_source_time.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native texture-source time requires MSVC Win32.
#endif

namespace bsp {
static_assert(sizeof(void*) == 4 && sizeof(float) == 4 && sizeof(double) == 8);

__declspec(naked) void __fastcall set_native_texture_source_time_00c302a0(
    void*, const void*, float) noexcept {
    __asm {
        sub esp, 8
        push esi
        push edi
        mov edi, edx
        mov esi, dword ptr [ecx + 14h]
        test esi, esi
        je time_complete
        fld dword ptr [esp + 14h]
        xor edx, edx
        fld dword ptr [ecx + 0Ch]
        fdivr qword ptr [edi]
        fdivp st(1), st(0)
        fnstcw word ptr [esp + 14h]
        movzx eax, word ptr [esp + 14h]
        or eax, 0C00h
        mov dword ptr [esp + 8], eax
        fldcw word ptr [esp + 8]
        fistp qword ptr [esp + 8]
        mov eax, dword ptr [esp + 8]
        div esi
        fldcw word ptr [esp + 14h]
        mov dword ptr [ecx + 8], edx
    time_complete:
        pop edi
        pop esi
        add esp, 8
        ret 4
    }
}

} // namespace bsp
