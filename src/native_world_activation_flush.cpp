#include "bsp/native_world_activation_flush.hpp"

#include "bsp/native_entity_destroy_state.hpp"

namespace bsp {

// Complete native 57-byte/22-instruction schedule. The only CALL binds to the
// concrete Source destroy-state helper. Six _emit bytes preserve the original
// LEA EBX,[EBX+00000000] without assembler displacement shortening.
__declspec(naked) void __fastcall flush_native_world_activations_00903670(void*) {
    __asm {
        mov eax, dword ptr [ecx + 4]         // 00903670
        push esi                            // 00903673
        mov esi, dword ptr [eax]             // 00903674
        test esi, esi                       // 00903676
        jz short activation_flush_done      // 00903678
        _emit 08dh                          // 0090367A
        _emit 09bh
        _emit 000h
        _emit 000h
        _emit 000h
        _emit 000h
    activation_flush_entity:
        cmp byte ptr [esi + 5eh], 0          // 00903680
        jz short activation_flush_next      // 00903684
        cmp dword ptr [esi + 6ch], 0         // 00903686
        jnz short activation_flush_next     // 0090368A
        mov eax, dword ptr [esi + 3ch]       // 0090368C
        test eax, eax                       // 0090368F
        jz short activation_flush_mark      // 00903691
        cmp byte ptr [eax + 5eh], 0          // 00903693
        jnz short activation_flush_next     // 00903697
    activation_flush_mark:
        mov ecx, esi                        // 00903699
        call mark_native_entity_destroy_state_00922fd0 // 0090369B
    activation_flush_next:
        mov esi, dword ptr [esi + 38h]       // 009036A0
        test esi, esi                       // 009036A3
        jnz short activation_flush_entity   // 009036A5
    activation_flush_done:
        pop esi                             // 009036A7
        ret                                 // 009036A8
    }
}

} // namespace bsp
