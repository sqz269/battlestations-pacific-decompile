#include "bsp/native_lua_variant_pair_advance.hpp"

#include "bsp/native_invalid_parameter_noinfo_call.hpp"

namespace bsp {

static_assert(sizeof(void*) == 4);

// Preserve all 38 physical operations, including the disp8(0) identity LEA.
// Both external transfers use the actual admitted Source entry. Its provider
// policy is qualified separately; final encoding/relocations are build-owned.
__declspec(naked) void __fastcall advance_native_lua_variant_pair_006eda20(void*) {
    __asm {
        push esi                              // 006EDA20
        mov esi, ecx                          // 006EDA21
        cmp dword ptr [esi], 0                // 006EDA23: full pair+0 guard
        jnz short pair_load_node              // 006EDA26
        call invoke_native_invalid_parameter_00bf6713 // 006EDA28: may return
    pair_load_node:
        mov eax, dword ptr [esi + 4]          // 006EDA2D: current pair+4
        cmp byte ptr [eax + 31h], 0           // 006EDA30
        jz short pair_select_right            // 006EDA34
        pop esi                               // 006EDA36: current saved word
        jmp invoke_native_invalid_parameter_00bf6713 // 006EDA37: restored-frame tail
    pair_select_right:
        mov ecx, dword ptr [eax + 8]          // 006EDA3C
        cmp byte ptr [ecx + 31h], 0           // 006EDA3F
        jnz short pair_ascend                  // 006EDA43
        mov eax, dword ptr [ecx]              // 006EDA45
        cmp byte ptr [eax + 31h], 0           // 006EDA47
        jnz short pair_store_descent          // 006EDA4B
        _emit 08dh                            // 006EDA4D: LEA ECX,[ECX+disp8(0)]
        _emit 049h
        _emit 000h
    pair_descend_loop:
        mov ecx, eax                          // 006EDA50
        mov eax, dword ptr [ecx]              // 006EDA52
        cmp byte ptr [eax + 31h], 0           // 006EDA54
        jz short pair_descend_loop             // 006EDA58
    pair_store_descent:
        mov dword ptr [esi + 4], ecx          // 006EDA5A
        pop esi                               // 006EDA5D
        ret                                   // 006EDA5E
    pair_ascend:
        mov eax, dword ptr [eax + 4]          // 006EDA5F
        cmp byte ptr [eax + 31h], 0           // 006EDA62
        jnz short pair_store_ascent           // 006EDA66
    pair_ascend_loop:
        mov ecx, dword ptr [esi + 4]          // 006EDA68: current pair word
        cmp ecx, dword ptr [eax + 8]          // 006EDA6B
        jnz short pair_store_ascent           // 006EDA6E
        mov dword ptr [esi + 4], eax          // 006EDA70: intermediate store
        mov edx, eax                          // 006EDA73
        mov eax, dword ptr [edx + 4]          // 006EDA75: read after that store
        cmp byte ptr [eax + 31h], 0           // 006EDA78
        jz short pair_ascend_loop              // 006EDA7C
    pair_store_ascent:
        mov dword ptr [esi + 4], eax          // 006EDA7E
        pop esi                               // 006EDA81
        ret                                   // 006EDA82
    }
}

} // namespace bsp
