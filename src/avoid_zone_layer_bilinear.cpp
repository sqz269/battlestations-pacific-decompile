#include "bsp/avoid_zone_layer_bilinear.hpp"

namespace bsp {

__declspec(naked) float __stdcall layer_sample_bilinear_kernel_0041bc20(
    Cc13TerrainGridLayerNative*, float, float) {
    // Entry takes the layer on the stack instead of native ECX. The native
    // dead argument slots still supply the h00/h10/blend/result scratch slots.
    // Each Source CellHeight call has one unused padding word so the native
    // local/scratch offsets survive its new EDX-x / stacked-z / RET4 ABI.
    __asm {
        mov ecx, dword ptr [esp + 4]
        sub esp, 18h
        fld dword ptr [esp + 24h] // Source world_z; native +20h
        push ebx
        push ebp
        push esi
        push edi
        sub esp, 8
        fstp dword ptr [esp + 4]
        lea edx, [esp + 28h]
        fld dword ptr [esp + 38h] // Source world_x; native +34h
        mov esi, ecx
        fstp dword ptr [esp]
        call layer_world_to_cell_kernel_00417fa0 // 0041BC40; Source RET8
        fld dword ptr [esp + 20h]
        cvttss2si edi, dword ptr [esp + 20h]
        cvttss2si ebx, dword ptr [esp + 24h]
        mov dword ptr [esp + 30h], edi
        fisub dword ptr [esp + 30h]
        lea ebp, [ebx + 1]
        lea eax, [edi + 1]
        sub esp, 4
        push ebp
        mov edx, eax
        mov dword ptr [esp + 24h], ebx
        fstp dword ptr [esp + 18h]
        mov ecx, esi // Source fastcall ECX is caller-volatile
        call layer_cell_height_kernel_0041bae0 // 0041BC6D: h11
        add esp, 4
        sub esp, 4
        fstp dword ptr [esp + 18h]
        push ebp
        mov edx, edi
        mov ecx, esi
        call layer_cell_height_kernel_0041bae0 // 0041BC7A: h01
        add esp, 4
        fstp dword ptr [esp + 18h]
        sub esp, 4
        lea eax, [edi + 1]
        push ebx
        mov edx, eax
        mov ecx, esi
        call layer_cell_height_kernel_0041bae0 // 0041BC8A: h10
        add esp, 4
        fstp dword ptr [esp + 30h]
        sub esp, 4
        push ebx
        mov edx, edi
        mov ecx, esi
        call layer_cell_height_kernel_0041bae0 // 0041BC97: h00
        add esp, 4
        fstp dword ptr [esp + 2ch]
        fld dword ptr [esp + 30h]
        pop edi
        fld dword ptr [esp + 28h]
        pop esi
        fld st(0)
        pop ebp
        fsubp st(2), st(0)
        pop ebx
        fld dword ptr [esp]
        fld st(0)
        fmulp st(3), st(0)
        fxch st(2)
        faddp st(1), st(0)
        fstp dword ptr [esp + 1ch]
        fld dword ptr [esp + 4]
        fld dword ptr [esp + 8]
        fld st(0)
        fsubp st(2), st(0)
        fxch
        fmulp st(2), st(0)
        faddp st(1), st(0)
        fstp dword ptr [esp + 20h]
        fld dword ptr [esp + 20h]
        fld dword ptr [esp + 1ch]
        fld st(0)
        fsubp st(2), st(0)
        fld dword ptr [esp + 14h]
        fisub dword ptr [esp + 0ch]
        fstp dword ptr [esp + 20h]
        fld dword ptr [esp + 20h]
        fmulp st(2), st(0)
        faddp st(1), st(0)
        fstp dword ptr [esp + 20h]
        fld dword ptr [esp + 20h]
        add esp, 18h
        ret 0ch
    }
}

} // namespace bsp
