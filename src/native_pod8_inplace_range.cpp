#include "bsp/native_pod8_inplace_range.hpp"

namespace bsp {

static_assert(sizeof(void*) == 4);

__declspec(naked) void __cdecl fill_native_pod8_range_0041c680(
    void*, const void*, const void*) {
    __asm {
        mov eax, dword ptr [esp + 4]
        mov edx, dword ptr [esp + 8]
        cmp eax, edx
        je finished
        mov ecx, dword ptr [esp + 0ch]
        push esi
    next_element:
        mov esi, dword ptr [ecx]
        mov dword ptr [eax], esi
        mov esi, dword ptr [ecx + 4]
        mov dword ptr [eax + 4], esi
        add eax, 8
        cmp eax, edx
        jne next_element
        pop esi
    finished:
        ret
    }
}

__declspec(naked) void* __cdecl copy_backward_native_pod8_range_0041dac0(
    const void*, const void*, void*) {
    __asm {
        mov ecx, dword ptr [esp + 8]
        mov edx, dword ptr [esp + 0ch]
        push esi
        mov esi, dword ptr [esp + 8]
        mov eax, ecx
        sub eax, esi
        sar eax, 3
        add eax, eax
        add eax, eax
        add eax, eax
        push edi
        mov edi, eax
        mov eax, edx
        sub eax, edi
        cmp esi, ecx
        je finished
        sub edx, ecx
    previous_element:
        mov edi, dword ptr [ecx - 8]
        sub ecx, 8
        cmp ecx, esi
        mov dword ptr [edx + ecx], edi
        mov edi, dword ptr [ecx + 4]
        mov dword ptr [edx + ecx + 4], edi
        jne previous_element
    finished:
        pop edi
        pop esi
        ret
    }
}

} // namespace bsp
