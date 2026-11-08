#include "bsp/native_ambient_color.hpp"
#include "bsp/native_render_batch_keys.hpp"
#include "bsp/cc13_sweepA.hpp"

namespace bsp {
namespace {
// Source-level reuse of 00412F60's signed integer clamp. The image inlines
// these same bounds at 004FB869..87D, 004FB88A..89E, 004FB8AB..8BF and
// 004FB8C9..8E5 (bsp.py show 004fb850 --asm). The argument is the real CRT
// conversion's low EAX, including its overflow result, never a float recast.
// This integer-only bridge must preserve the live x87 scale between conversions.
__declspec(noinline) std::int32_t __fastcall clamp_ambient_channel(std::int32_t value) {
    const std::int32_t low = 0;
    const std::int32_t high = 255;
    return cc13_sweepA::int_clamp_00412f60(&value, &low, &high);
}
} // namespace

__declspec(naked) const void* __fastcall native_ambient_color_address_00b84c60(
    const void*) noexcept {
    __asm { lea eax, [ecx + 8] }
    __asm { ret }
}

__declspec(naked) void* __fastcall convert_native_float_rgba_to_argb_004fb850(
    void*, const void*, const volatile double*, const volatile std::uint32_t*) {
    __asm {
        push esi
        push edi
        mov edi, edx
        fld dword ptr [edi]
        mov esi, ecx
        mov eax, dword ptr [esp + 0ch]
        fld qword ptr [eax]
        // DC C9 multiplies ST1 by ST0, retaining the original scale in ST0.
        fmul st(1), st(0)
        fxch st(1)
        mov ecx, dword ptr [esp + 10h]
        call native_crt_truncate_st0_00bf7420
        mov ecx, eax
        call clamp_ambient_channel
        mov byte ptr [esi + 2], al
        fld dword ptr [edi + 4]
        fmul st(0), st(1)
        mov ecx, dword ptr [esp + 10h]
        call native_crt_truncate_st0_00bf7420
        mov ecx, eax
        call clamp_ambient_channel
        mov byte ptr [esi + 1], al
        fld dword ptr [edi + 8]
        fmul st(0), st(1)
        mov ecx, dword ptr [esp + 10h]
        call native_crt_truncate_st0_00bf7420
        mov ecx, eax
        call clamp_ambient_channel
        mov byte ptr [esi], al
        fmul dword ptr [edi + 0ch]
        mov ecx, dword ptr [esp + 10h]
        call native_crt_truncate_st0_00bf7420
        mov ecx, eax
        call clamp_ambient_channel
        mov byte ptr [esi + 3], al
        pop edi
        mov eax, esi
        pop esi
        ret 8
    }
}

} // namespace bsp
