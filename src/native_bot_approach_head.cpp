#include "bsp/native_bot_approach_head.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native bot approach head reconstruction requires MSVC Win32.
#endif

namespace bsp {

NativeBotApproachHeadStorage* construct_native_bot_approach_head_009f9ce0(
    NativeBotApproachHeadStorage* head, const void* unit, float reference_speed,
    const NativeBotApproachHeadContext& context) noexcept {
    // Address formation only: values are read at the original instruction sites.
    const void* const volatile* const rows_cell = &context.descriptor_rows_00f8a30c;
    const volatile float* const fallback_cell = &context.fallback_one_00d7a24c;
    const volatile float* const minus_cell = &context.minus_one_00d7a260;
    float ratio_spill;
    __asm {
        mov ecx, head
        mov eax, unit
        mov dword ptr [ecx], 0d21c74h
        mov dword ptr [ecx + 4], eax
        mov edx, dword ptr [eax + 538h]
        mov dword ptr [ecx + 8], edx
        mov edx, dword ptr [eax + 9d4h]
        mov dword ptr [ecx + 0ch], edx
        mov edx, dword ptr [eax + 0df4h]
        mov dword ptr [ecx + 10h], edx
        mov edx, dword ptr [eax + 0df4h]
        mov edx, dword ptr [edx + 34h]
        imul edx, edx, 248h
        mov esi, rows_cell
        mov esi, dword ptr [esi]
        lea edx, [edx + esi + 0ch]
        mov dword ptr [ecx + 14h], edx
        xor edx, edx
        mov dword ptr [ecx + 18h], edx
        mov dword ptr [ecx + 1ch], edx
        mov dword ptr [ecx + 20h], edx
        mov eax, dword ptr [eax + 538h]
        fld dword ptr [eax + 188h]
        fdiv reference_speed
        fstp ratio_spill
        fld1
        fld ratio_spill
        fcomip st(0), st(1)
        fstp st(0)
        jbe fallback
        movss xmm0, ratio_spill
        jmp publish_ratio
    fallback:
        mov edx, fallback_cell
        movss xmm0, dword ptr [edx]
    publish_ratio:
        movss dword ptr [ecx + 24h], xmm0
        mov edx, minus_cell
        movss xmm0, dword ptr [edx]
        movss dword ptr [ecx + 28h], xmm0
    }
    return head;
}

NativeBotApproachHeadHost::NativeBotApproachHeadHost(
    const NativeBotApproachHeadContext& context) noexcept : head_context_(context) {}

void NativeBotApproachHeadHost::construct_speed_reference(
    void* head, const void* unit, float reference_speed) {
    (void)construct_native_bot_approach_head_009f9ce0(
        static_cast<NativeBotApproachHeadStorage*>(head), unit, reference_speed, head_context_);
}

} // namespace bsp
