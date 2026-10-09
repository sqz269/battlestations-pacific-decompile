#include "bsp/native_world_entity_chain_retirement.hpp"

#if defined(_MSC_VER) && defined(_M_IX86)

namespace bsp {

__declspec(naked) void __fastcall
retire_native_world_entity_chain_009041a0(void*) {
    __asm {
        push esi                       // 009041A0
        push edi                       // 009041A1
        mov esi, ecx                   // 009041A2
        xor edi, edi                   // 009041A4
        cmp dword ptr [esi + 8], edi    // 009041A6
        jz done                        // 009041A9
        jmp entity_loop                // 009041AB
        // Unreachable native three-byte alignment LEA ECX,[ECX+0].
        _emit 08dh                     // 009041AD
        _emit 049h
        _emit 000h

    entity_loop:
        mov ecx, dword ptr [esi]        // 009041B0
        mov eax, dword ptr [ecx + 34h]  // 009041B2
        cmp eax, edi                   // 009041B5
        jnz unlink_previous            // 009041B7
        cmp dword ptr [ecx + 38h], edi  // 009041B9
        jnz check_previous             // 009041BC
        cmp dword ptr [esi + 8], 1      // 009041BE
        jg release_own                 // 009041C2

    check_previous:
        cmp eax, edi                   // 009041C4
        jz replace_first               // 009041C6
    unlink_previous:
        mov edx, dword ptr [ecx + 38h]  // 009041C8
        mov dword ptr [eax + 38h], edx  // 009041CB
        jmp check_next                 // 009041CE
    replace_first:
        mov eax, dword ptr [ecx + 38h]  // 009041D0
        mov dword ptr [esi], eax        // 009041D3
    check_next:
        mov eax, dword ptr [ecx + 38h]  // 009041D5
        cmp eax, edi                   // 009041D8
        jz replace_last                // 009041DA
        mov edx, dword ptr [ecx + 34h]  // 009041DC
        mov dword ptr [eax + 34h], edx  // 009041DF
        jmp clear_links                // 009041E2
    replace_last:
        mov eax, dword ptr [ecx + 34h]  // 009041E4
        mov dword ptr [esi + 4], eax    // 009041E7
    clear_links:
        mov dword ptr [ecx + 38h], edi  // 009041EA
        mov dword ptr [ecx + 34h], edi  // 009041ED
        add dword ptr [esi + 8], -1     // 009041F0
    release_own:
        mov edx, dword ptr [ecx]        // 009041F4
        mov eax, dword ptr [edx]        // 009041F6
        push 1                         // 009041F8
        call eax                       // 009041FA
        cmp dword ptr [esi + 8], edi    // 009041FC
        jnz entity_loop                // 009041FF
    done:
        pop edi                        // 00904201
        pop esi                        // 00904202
        ret                            // 00904203
    }
}

} // namespace bsp

#endif
