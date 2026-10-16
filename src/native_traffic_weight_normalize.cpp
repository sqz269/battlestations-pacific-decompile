#include "bsp/native_traffic_weight_normalize.hpp"

#include "bsp/native_invalid_parameter_noinfo_call.hpp"

namespace bsp {

static_assert(sizeof(void*) == 4);

__declspec(naked) std::uint32_t __fastcall
size_native_traffic_dwords_0043ec30(const void*) {
    __asm {
        mov edx, dword ptr [ecx + 4]         // 0043EC30
        test edx, edx
        jnz size_nonnull
        xor eax, eax
        ret
    size_nonnull:
        mov eax, dword ptr [ecx + 8]
        sub eax, edx
        sar eax, 2
        ret                                 // 0043EC42
    }
}

__declspec(naked) std::uint32_t __fastcall
normalize_native_traffic_weights_004809f0(void*) {
    __asm {
        push ecx                            // 004809F0: actual S-4 sum cell
        xorps xmm0, xmm0
        push esi
        push edi
        mov esi, ecx
        movss dword ptr [esp + 8], xmm0
        xor edi, edi
        call size_native_traffic_dwords_0043ec30 // 00480A00
        test eax, eax
        jbe accumulation_done
        lea esp, [esp + 00000000h]           // 00480A09: same LEA, assembler-sized
    accumulate:
        mov ecx, dword ptr [esi + 4]         // 00480A10
        test ecx, ecx
        jz accumulation_diagnostic
        mov eax, dword ptr [esi + 8]
        sub eax, ecx
        sar eax, 2
        cmp edi, eax
        jb accumulation_load
    accumulation_diagnostic:
        call invoke_native_invalid_parameter_00bf6713 // 00480A23
    accumulation_load:
        mov eax, dword ptr [esi + 4]
        fld dword ptr [eax + edi * 4]
        mov ecx, esi
        fadd dword ptr [esp + 8]
        add edi, 1
        fstp dword ptr [esp + 8]
        call size_native_traffic_dwords_0043ec30 // 00480A3B
        cmp edi, eax
        jb accumulate
    accumulation_done:
        mov ecx, esi                        // 00480A44
        xor edi, edi
        call size_native_traffic_dwords_0043ec30 // 00480A48
        test eax, eax
        jbe normalization_done
    normalize:
        mov ecx, dword ptr [esi + 4]
        test ecx, ecx
        jz normalization_diagnostic
        mov eax, dword ptr [esi + 8]
        sub eax, ecx
        sar eax, 2
        cmp edi, eax
        jb normalization_load
    normalization_diagnostic:
        call invoke_native_invalid_parameter_00bf6713 // 00480A64
    normalization_load:
        mov ecx, dword ptr [esi + 4]
        fld dword ptr [ecx + edi * 4]
        lea eax, [ecx + edi * 4]
        fdiv dword ptr [esp + 8]
        mov ecx, esi
        add edi, 1
        fstp dword ptr [eax]
        call size_native_traffic_dwords_0043ec30 // 00480A7D
        cmp edi, eax
        jb normalize
    normalization_done:
        pop edi                             // 00480A86
        pop esi
        pop ecx                             // CURRENT S-4 raw word
        ret
    }
}

} // namespace bsp
