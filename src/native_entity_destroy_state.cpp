#include "bsp/native_entity_destroy_state.hpp"

namespace bsp {

// Complete native 66-byte/25-instruction schedule. Only the recursive CALL
// relocates to this real Source entry. The three _emit bytes retain the native
// LEA ECX,[ECX+00] instruction without assembler displacement shortening.
__declspec(naked) void __fastcall mark_native_entity_destroy_state_00922fd0(void*) {
    __asm {
        push esi                            // 00922FD0
        push edi                            // 00922FD1
        mov edi, ecx                        // 00922FD2
        mov esi, dword ptr [edi + 48h]       // 00922FD4
        test esi, esi                       // 00922FD7
        mov eax, 1                          // 00922FD9
        mov byte ptr [edi + 5eh], al         // 00922FDE
        mov byte ptr [edi + 5dh], al         // 00922FE1
        mov byte ptr [edi + 5ch], 0          // 00922FE4
        mov dword ptr [edi + 6ch], eax       // 00922FE8
        jz short destroy_state_dispatch     // 00922FEB
        _emit 08dh                          // 00922FED
        _emit 049h
        _emit 000h
    destroy_state_child:
        cmp dword ptr [esi + 6ch], 0         // 00922FF0
        jnz short destroy_state_next        // 00922FF4
        mov ecx, esi                        // 00922FF6
        call mark_native_entity_destroy_state_00922fd0 // 00922FF8
    destroy_state_next:
        mov esi, dword ptr [esi + 44h]       // 00922FFD
        test esi, esi                       // 00923000
        jnz short destroy_state_child       // 00923002
    destroy_state_dispatch:
        mov eax, dword ptr [edi]             // 00923004
        mov edx, dword ptr [eax + 84h]       // 00923006
        mov ecx, edi                        // 0092300C
        pop edi                             // 0092300E
        pop esi                             // 0092300F
        jmp edx                             // 00923010
    }
}

} // namespace bsp
