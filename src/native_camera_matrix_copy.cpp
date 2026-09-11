#include "bsp/native_camera_matrix_copy.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native camera matrix copying requires MSVC Win32 x87 assembly.
#endif

namespace bsp {
// Complete 004134F0..00413557. Keep the original load/store order and
// ambient x87 state; overlapping inputs are not copied to a temporary.
__declspec(naked) void* __fastcall copy_native_camera_matrix_004134f0(
    void*, void*, const void*) {
    __asm {
        mov eax, ecx
        mov ecx, dword ptr [esp + 4]
        fld dword ptr [ecx]
        fstp dword ptr [eax]
        fld dword ptr [ecx + 4]
        fstp dword ptr [eax + 4]
        fld dword ptr [ecx + 8]
        fstp dword ptr [eax + 8]
        fld dword ptr [ecx + 0ch]
        fstp dword ptr [eax + 0ch]
        fld dword ptr [ecx + 10h]
        fstp dword ptr [eax + 10h]
        fld dword ptr [ecx + 14h]
        fstp dword ptr [eax + 14h]
        fld dword ptr [ecx + 18h]
        fstp dword ptr [eax + 18h]
        fld dword ptr [ecx + 1ch]
        fstp dword ptr [eax + 1ch]
        fld dword ptr [ecx + 20h]
        fstp dword ptr [eax + 20h]
        fld dword ptr [ecx + 24h]
        fstp dword ptr [eax + 24h]
        fld dword ptr [ecx + 28h]
        fstp dword ptr [eax + 28h]
        fld dword ptr [ecx + 2ch]
        fstp dword ptr [eax + 2ch]
        fld dword ptr [ecx + 30h]
        fstp dword ptr [eax + 30h]
        fld dword ptr [ecx + 34h]
        fstp dword ptr [eax + 34h]
        fld dword ptr [ecx + 38h]
        fstp dword ptr [eax + 38h]
        fld dword ptr [ecx + 3ch]
        fstp dword ptr [eax + 3ch]
        ret 4
    }
}
} // namespace bsp
