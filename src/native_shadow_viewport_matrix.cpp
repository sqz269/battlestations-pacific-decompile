#include "bsp/native_shadow_viewport_matrix.hpp"
#include "bsp/native_renderer_viewport_clear.hpp"

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native shadow viewport matrix requires MSVC Win32.
#endif

namespace bsp {
namespace {
// Original PE .rdata is readable and not writable (0x40000040). Keep exact
// representations and memory operand widths; these are not computed values.
const std::uint32_t unsigned_correction_00ce3978 = 0x4f800000u;
alignas(8) const std::uint64_t half_00d7a280 = 0x3fe0000000000000ull;
const std::uint32_t one_00d7a24c = 0x3f800000u;
}

__declspec(naked) void* __fastcall build_native_shadow_viewport_matrix_00a8aaa0(
    const void*, void*, void*, const void*) noexcept {
    __asm {
        sub esp, 10h
        push esi
        push edi
        mov edi, dword ptr [esp + 20h]
        mov esi, ecx
        mov ecx, edi
        call native_viewport_size_00b1f740
        fild dword ptr [eax]
        mov eax, dword ptr [esi + 388h]
        fild dword ptr [esi + 388h]
        test eax, eax
        jge width_ratio_unsigned
        fadd dword ptr [unsigned_correction_00ce3978]
    width_ratio_unsigned:
        fdivp st(1), st(0)
        mov ecx, edi
        fstp dword ptr [esp + 20h]
        call native_viewport_size_00b1f740
        fild dword ptr [eax + 4]
        mov ecx, dword ptr [esi + 38ch]
        fild dword ptr [esi + 38ch]
        test ecx, ecx
        jge height_ratio_unsigned
        fadd dword ptr [unsigned_correction_00ce3978]
    height_ratio_unsigned:
        fdivp st(1), st(0)
        mov ecx, edi
        fstp dword ptr [esp + 0ch]
        call native_viewport_origin_00b1f730
        fild dword ptr [esi + 388h]
        mov edx, dword ptr [esi + 388h]
        test edx, edx
        jge origin_width_unsigned
        fadd dword ptr [unsigned_correction_00ce3978]
    origin_width_unsigned:
        fstp dword ptr [esp + 8]
        fld dword ptr [esp + 20h]
        fld qword ptr [half_00d7a280]
        fmul st(1), st(0) // DC C9: destination ST1, not ST0.
        fxch st(1)
        fst qword ptr [esp + 10h]
        fild dword ptr [eax]
        mov eax, dword ptr [esi + 38ch]
        test eax, eax
        fld dword ptr [esp + 8]
        fld st(0)
        fdivp st(2), st(0)
        fxch st(1)
        faddp st(2), st(0)
        fdivp st(2), st(0)
        faddp st(1), st(0)
        fstp dword ptr [esp + 8]
        fild dword ptr [esi + 38ch]
        jge origin_height_unsigned
        fadd dword ptr [unsigned_correction_00ce3978]
    origin_height_unsigned:
        mov ecx, edi
        fstp dword ptr [esp + 20h]
        call native_viewport_origin_00b1f730
        fild dword ptr [eax + 4]
        fld dword ptr [esp + 20h]
        mov eax, dword ptr [esp + 1ch]
        fld st(0)
        xorps xmm0, xmm0
        fdivp st(2), st(0)
        movss xmm2, dword ptr [esp + 8]
        movss xmm1, dword ptr [one_00d7a24c]
        movss dword ptr [eax + 30h], xmm2
        pop edi
        movss dword ptr [eax + 4], xmm0
        movss dword ptr [eax + 8], xmm0
        movss dword ptr [eax + 0ch], xmm0
        movss dword ptr [eax + 10h], xmm0
        movss dword ptr [eax + 18h], xmm0
        movss dword ptr [eax + 1ch], xmm0
        movss dword ptr [eax + 20h], xmm0
        movss dword ptr [eax + 24h], xmm0
        movss dword ptr [eax + 28h], xmm1
        movss dword ptr [eax + 2ch], xmm0
        movss dword ptr [eax + 38h], xmm0
        movss dword ptr [eax + 3ch], xmm1
        pop esi
        fld dword ptr [esp + 4]
        fld st(0)
        fld qword ptr [half_00d7a280]
        fmul st(1), st(0) // DC C9 again.
        fxch st(4)
        faddp st(1), st(0)
        fld st(3)
        fdivrp st(3), st(0)
        faddp st(2), st(0)
        fxch st(1)
        fstp dword ptr [esp + 18h]
        fld qword ptr [esp + 8]
        movss xmm2, dword ptr [esp + 18h]
        fstp dword ptr [eax]
        movss dword ptr [eax + 34h], xmm2
        fchs
        fmulp st(1), st(0)
        fstp dword ptr [eax + 14h]
        add esp, 10h
        ret 8
    }
}

__declspec(naked) void* __fastcall native_shadow_target_field14_00a8fda0(
    const void*) noexcept {
    __asm { mov eax, dword ptr [ecx + 14h] }
    __asm { ret }
}

__declspec(naked) void* __fastcall native_shadow_target_field1c_00a8fdc0(
    const void*) noexcept {
    __asm { mov eax, dword ptr [ecx + 1ch] }
    __asm { ret }
}
} // namespace bsp
