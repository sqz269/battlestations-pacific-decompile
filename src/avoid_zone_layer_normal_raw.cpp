#include "bsp/avoid_zone_layer_normal_raw.hpp"
#include "bsp/avoid_zone_layer_sampling.hpp"
#include "bsp/native_render_batch_keys.hpp"

namespace bsp {
namespace {
const double kNormalNeighborStep = 1.0; // 00D7A210: 00 00 00 00 00 00 F0 3F
} // namespace

__declspec(naked) float* __stdcall layer_normal_raw_kernel_0041bd10(
    float*, const float*, Cc13TerrainGridLayerNative*, const volatile std::uint32_t*,
    const CameraAxesCrtAccess*) {
    __asm {
        mov ecx, dword ptr [esp + 0ch] // additional Source layer argument
        sub esp, 20h
        mov eax, dword ptr [esp + 28h]
        fld dword ptr [eax + 4]
        push ebx
        push ebp
        push esi
        push edi
        sub esp, 8
        fstp dword ptr [esp + 4]
        mov edi, ecx
        fld dword ptr [eax]
        lea edx, [esp + 20h]
        fstp dword ptr [esp]
        call layer_world_to_cell_kernel_00417fa0 // 0041BD31; Source RET8
        cvttss2si esi, dword ptr [esp + 1ch]
        cvttss2si ebx, dword ptr [esp + 18h]
        sub esp, 4 // Source height padding; preserve native scratch offsets
        push esi
        mov edx, ebx
        mov ecx, edi
        call layer_cell_height_kernel_0041bae0 // 0041BD44: h00
        add esp, 4
        fstp dword ptr [esp + 38h]
        fld dword ptr [esp + 18h]
        fadd kNormalNeighborStep
        mov ecx, dword ptr [esp + 40h] // actual mutable0109EEA4 address
        call native_crt_truncate_st0_00bf7420 // 0041BD57
        mov ebp, eax
        sub esp, 4
        push esi
        mov edx, ebp
        mov ecx, edi
        call layer_cell_height_kernel_0041bae0 // 0041BD62: h10
        add esp, 4
        fstp dword ptr [esp + 14h]
        fld dword ptr [esp + 1ch]
        fadd kNormalNeighborStep
        mov ecx, dword ptr [esp + 40h]
        call native_crt_truncate_st0_00bf7420 // 0041BD75
        sub esp, 4
        push eax
        mov edx, ebx
        mov ecx, edi
        mov dword ptr [esp + 20h], eax
        call layer_cell_height_kernel_0041bae0 // 0041BD82: h01
        add esp, 4
        fstp dword ptr [esp + 10h]
        fld dword ptr [esp + 38h]
        movss xmm0, dword ptr [edi + 0ch]
        mov esi, dword ptr [esp + 34h]
        fld st(0)
        fsub dword ptr [esp + 14h]
        mov ecx, dword ptr [esp + 18h]
        sub esp, 4
        push ecx
        mov edx, ebp
        fstp dword ptr [esi]
        mov ecx, edi
        movss dword ptr [esi + 4], xmm0
        fsub dword ptr [esp + 18h]
        fstp dword ptr [esi + 8]
        call layer_cell_height_kernel_0041bae0 // 0041BDB4: h11 after stores
        add esp, 4
        fstp dword ptr [esp + 38h]
        mov ecx, esi
        fld dword ptr [esp + 10h]
        fld dword ptr [esp + 38h]
        fld st(0)
        fsubp st(2), st(0)
        fxch
        fstp dword ptr [esp + 24h]
        fld dword ptr [edi + 0ch]
        fstp dword ptr [esp + 28h]
        fsubr dword ptr [esp + 14h]
        fstp dword ptr [esp + 2ch]
        fld dword ptr [esi]
        fadd dword ptr [esp + 24h]
        fstp dword ptr [esp + 18h]
        fld dword ptr [esi + 4]
        fadd dword ptr [esp + 28h]
        fstp dword ptr [esp + 1ch]
        fld dword ptr [esi + 8]
        fadd dword ptr [esp + 2ch]
        fstp dword ptr [esp + 20h]
        fld dword ptr [esp + 18h]
        fstp dword ptr [esi]
        fld dword ptr [esp + 1ch]
        fstp dword ptr [esi + 4]
        fld dword ptr [esp + 20h]
        fstp dword ptr [esi + 8]
        mov edx, dword ptr [esp + 44h] // additional Source CRT access
        call camera_vector_length_00419440 // 0041BE14; finite/diagnostic boundary
        fstp dword ptr [esp + 38h]
        fldz
        fld dword ptr [esp + 38h]
        fcomi st(0), st(1)
        fstp st(1)
        jbe zero_reciprocal
        fld1
        fdivrp st(1), st(0)
        fstp dword ptr [esp + 38h]
        jmp scale_gradient
    zero_reciprocal:
        xorps xmm0, xmm0
        fstp st(0)
        movss dword ptr [esp + 38h], xmm0
    scale_gradient:
        fld dword ptr [esp + 18h]
        pop edi
        fld dword ptr [esp + 34h]
        mov eax, esi
        fld st(0)
        fmulp st(2), st(0)
        fxch
        fstp dword ptr [esp + 20h]
        fld dword ptr [esp + 18h]
        fmul st(0), st(1)
        fstp dword ptr [esp + 24h]
        fmul dword ptr [esp + 1ch]
        fstp dword ptr [esp + 28h]
        fld dword ptr [esp + 20h]
        fstp dword ptr [esi]
        fld dword ptr [esp + 24h]
        fstp dword ptr [esi + 4]
        fld dword ptr [esp + 28h]
        fstp dword ptr [esi + 8]
        pop esi
        pop ebp
        pop ebx
        add esp, 20h
        ret 14h
    }
}

} // namespace bsp
