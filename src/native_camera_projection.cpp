#include "bsp/native_camera_projection.hpp"

namespace bsp {
namespace {
// Exact verified immutable original .rdata bits; these addresses relocate.
const std::uint64_t projection_half_d7a280 = 0x3fe0000000000000ull;
const std::uint32_t projection_one_d7a24c = 0x3f800000u;
}

__declspec(naked) float __stdcall native_projection_tangent_00412e20(
    std::uint32_t) {
    __asm {
        push ecx
        fld dword ptr [esp + 8]
        fsincos
        fdivp st(1), st(0)
        fstp dword ptr [esp]
        fld dword ptr [esp]
        pop ecx
        ret 4
    }
}

__declspec(naked) void* __fastcall build_native_camera_projection_00b642f0(
    void*, void*) {
    __asm {
        fld dword ptr [edx]
        push esi
        fmul qword ptr [projection_half_d7a280]
        push ecx
        mov esi, ecx
        fstp dword ptr [edx]
        fld dword ptr [edx]
        fstp dword ptr [esp]
        call native_projection_tangent_00412e20
        // The complete helper above preserves EDX, our raw scalar-slot base.
        fld1
        xorps xmm0, xmm0
        fdivrp st(1), st(0)
        movss dword ptr [esi + 4], xmm0
        movss dword ptr [esi + 8], xmm0
        movss dword ptr [esi + 0ch], xmm0
        movss dword ptr [esi + 10h], xmm0
        movss dword ptr [esi + 18h], xmm0
        movss dword ptr [esi + 1ch], xmm0
        movss dword ptr [esi + 20h], xmm0
        movss dword ptr [esi + 24h], xmm0
        movss dword ptr [esi + 30h], xmm0
        movss dword ptr [esi + 34h], xmm0
        movss dword ptr [esi + 3ch], xmm0
        mov eax, esi
        fstp dword ptr [edx]
        fld dword ptr [edx + 0ch]
        fld dword ptr [edx + 8]
        fld st(0)
        movss xmm1, dword ptr [edx]
        fsubr st(0), st(2)
        movss dword ptr [esi + 14h], xmm1
        fdivp st(2), st(0)
        fxch st(1)
        fstp dword ptr [edx + 0ch]
        fld dword ptr [edx]
        fdiv dword ptr [edx + 4]
        movss xmm1, dword ptr [edx + 0ch]
        movss dword ptr [esi + 28h], xmm1
        movss xmm1, dword ptr [projection_one_d7a24c]
        movss dword ptr [esi + 2ch], xmm1
        fstp dword ptr [esi]
        fchs
        fmul dword ptr [edx + 0ch]
        fstp dword ptr [esi + 38h]
        pop esi
        ret
    }
}

} // namespace bsp
