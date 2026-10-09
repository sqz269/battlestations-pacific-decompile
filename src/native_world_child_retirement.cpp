#include "bsp/native_world_child_retirement.hpp"

#if defined(_MSC_VER) && defined(_M_IX86)

namespace bsp {

__declspec(naked) void __fastcall
retire_native_world_child_subtree_009035e0(void*) {
    __asm {
        push esi                       // 009035E0
        mov esi, ecx                   // 009035E1
        cmp dword ptr [esi + 50h], 0    // 009035E3
        jz own_dispatch                // 009035E7

        // 009035E9: preserve the native seven-byte LEA ESP,[ESP+00000000].
        _emit 08dh
        _emit 0a4h
        _emit 024h
        _emit 000h
        _emit 000h
        _emit 000h
        _emit 000h

    child_loop:
        mov ecx, dword ptr [esi + 48h]  // 009035F0
        call retire_native_world_child_subtree_009035e0 // 009035F3
        cmp dword ptr [esi + 50h], 0    // 009035F8
        jnz child_loop                 // 009035FC

    own_dispatch:
        mov eax, dword ptr [esi]        // 009035FE
        mov edx, dword ptr [eax]        // 00903600
        push 1                         // 00903602
        mov ecx, esi                   // 00903604
        call edx                       // 00903606
        pop esi                        // 00903608
        ret                            // 00903609
    }
}

} // namespace bsp

#endif
