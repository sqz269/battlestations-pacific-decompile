#include "bsp/native_lua_variant_pair_retreat.hpp"

#include "bsp/native_invalid_parameter_noinfo_call.hpp"

namespace bsp {

static_assert(sizeof(void*) == 4);

// Preserve all 48 physical operations, including both disp32(0) identity
// LEA ESP encodings. Only the three external REL32 operands are redirected
// to the actual admitted Source provider. Compiled bytes remain build-owned.
__declspec(naked) void __fastcall retreat_native_lua_variant_pair_006edd80(void*) {
    __asm {
        push esi                              // 006EDD80
        mov esi, ecx                          // 006EDD81
        cmp dword ptr [esi], 0                // 006EDD83: full pair+0 guard
        jnz short pair_load_node              // 006EDD86
        call invoke_native_invalid_parameter_00bf6713 // 006EDD88: may return
    pair_load_node:
        mov eax, dword ptr [esi + 4]          // 006EDD8D: current pair+4
        cmp byte ptr [eax + 31h], 0           // 006EDD90
        jz short pair_select_left             // 006EDD94
        mov eax, dword ptr [eax + 8]          // 006EDD96
        mov dword ptr [esi + 4], eax          // 006EDD99: store before flag read
        cmp byte ptr [eax + 31h], 0           // 006EDD9C
        jz short pair_return                  // 006EDDA0
        pop esi                               // 006EDDA2: current saved word
        jmp invoke_native_invalid_parameter_00bf6713 // 006EDDA3: restored-frame tail
    pair_select_left:
        mov ecx, dword ptr [eax]              // 006EDDA8
        cmp byte ptr [ecx + 31h], 0           // 006EDDAA
        jnz short pair_ascend                  // 006EDDAE
        mov eax, dword ptr [ecx + 8]          // 006EDDB0
        cmp byte ptr [eax + 31h], 0           // 006EDDB3
        jnz short pair_store_descent          // 006EDDB7
        _emit 08dh                            // 006EDDB9: LEA ESP,[ESP+disp32(0)]
        _emit 0a4h
        _emit 024h
        _emit 000h
        _emit 000h
        _emit 000h
        _emit 000h
    pair_descend_loop:
        mov ecx, eax                          // 006EDDC0
        mov eax, dword ptr [ecx + 8]          // 006EDDC2
        cmp byte ptr [eax + 31h], 0           // 006EDDC5
        jz short pair_descend_loop             // 006EDDC9
    pair_store_descent:
        mov dword ptr [esi + 4], ecx          // 006EDDCB
        pop esi                               // 006EDDCE
        ret                                   // 006EDDCF
    pair_ascend:
        mov eax, dword ptr [eax + 4]          // 006EDDD0
        cmp byte ptr [eax + 31h], 0           // 006EDDD3
        jnz short pair_validate_current       // 006EDDD7
        _emit 08dh                            // 006EDDD9: LEA ESP,[ESP+disp32(0)]
        _emit 0a4h
        _emit 024h
        _emit 000h
        _emit 000h
        _emit 000h
        _emit 000h
    pair_ascend_loop:
        mov ecx, dword ptr [esi + 4]          // 006EDDE0: current pair word
        cmp ecx, dword ptr [eax]              // 006EDDE3
        jnz short pair_validate_current       // 006EDDE5
        mov dword ptr [esi + 4], eax          // 006EDDE7: intermediate store
        mov edx, eax                          // 006EDDEA
        mov eax, dword ptr [edx + 4]          // 006EDDEC: load after that store
        cmp byte ptr [eax + 31h], 0           // 006EDDEF
        jz short pair_ascend_loop              // 006EDDF3
    pair_validate_current:
        mov ecx, dword ptr [esi + 4]          // 006EDDF5: current pair, not EAX
        cmp byte ptr [ecx + 31h], 0           // 006EDDF8
        jz short pair_store_ascent            // 006EDDFC
        pop esi                               // 006EDDFE: current saved word
        jmp invoke_native_invalid_parameter_00bf6713 // 006EDDFF: restored-frame tail
    pair_store_ascent:
        mov dword ptr [esi + 4], eax          // 006EDE04
    pair_return:
        pop esi                               // 006EDE07
        ret                                   // 006EDE08
    }
}

} // namespace bsp
