#include "bsp/native_input_scalar_tree_iterator_equal.hpp"
#include "bsp/native_invalid_parameter_noinfo_call.hpp"

namespace bsp {
static_assert(sizeof(void*) == 4);

__declspec(naked) bool __fastcall equal_native_input_scalar_tree_iterators_00553070(
    void*, void*, const void*) {
    __asm {
        push esi
        mov esi, ecx
        mov eax, dword ptr [esi]
        test eax, eax
        push edi
        mov edi, dword ptr [esp + 0ch]
        jz L_invalid_00553082
        cmp eax, dword ptr [edi]
        jz L_nodes_00553087
    L_invalid_00553082:
        call invoke_native_invalid_parameter_00bf6713
    L_nodes_00553087:
        mov eax, dword ptr [esi + 4]
        xor ecx, ecx
        cmp eax, dword ptr [edi + 4]
        pop edi
        setz cl
        mov al, cl
        pop esi
        ret 4
    }
}
} // namespace bsp
