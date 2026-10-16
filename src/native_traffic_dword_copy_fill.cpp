#include "bsp/native_traffic_dword_copy_fill.hpp"

#include <cstring>

namespace bsp {

static_assert(sizeof(void*) == 4);

__declspec(naked) void* __cdecl fill_native_traffic_dwords_00486800(
    void*, const void*, const void*) {
    __asm {
        mov eax,[esp+0x4] //00486800: first actual stack cell
        mov ecx,[esp+0x8] //00486804: captured endpoint
        cmp eax,ecx //00486808
        jz fill_range_done //0048680A: do not read third cell when empty
        mov edx,[esp+0xc] //0048680C: capture value-cell address once
        push esi //00486810
    fill_range_loop:
        mov esi,[edx] //00486811: reread actual borrowed DWORD
        mov [eax],esi //00486813
        add eax,0x4 //00486815: DWORD wrap
        cmp eax,ecx //00486818: exact equality, no ordered-range repair
        jnz fill_range_loop //0048681A
        pop esi //0048681C
    fill_range_done:
        ret //0048681D: EAX endpoint; caller pops three words
    }
}

__declspec(naked) void* __cdecl copy_native_traffic_dwords_backward_00486820(
    const void*, const void*, void*) {
    __asm {
        mov eax,[esp+0x8] //00486820: last
        mov edx,[esp+0x4] //00486824: first
        sub eax,edx //00486828: wrapped DWORD difference
        sar eax,0x2 //0048682A: signed count
        push esi //0048682D
        mov esi,[esp+0x10] //0048682E: actual destination-end cell
        lea ecx,[eax*0x4] //00486832: wrapped byte span
        sub esi,ecx //00486839: captured destination begin
        test eax,eax //0048683B
        jle copy_backward_done //0048683D: signed positive only
        push ecx //0048683F: count bytes
        push edx //00486840: first/source
        push ecx //00486841: actual destination capacity
        push esi //00486842: actual destination
        call memmove_s //00486843: genuine selected SDK C API
        add esp,0x10 //00486848
    copy_backward_done:
        mov eax,esi //0048684B: captured result even if service returns error
        pop esi //0048684D
        ret //0048684E: caller pops three words
    }
}

__declspec(naked) void* __stdcall copy_native_traffic_dwords_0048d660(
    const void*, const void*, void*) {
    __asm {
        mov eax,[esp+0x8] //0048D660: last
        mov ecx,[esp+0x4] //0048D664: first
        mov edx,[esp+0xc] //0048D668: actual destination
        sub eax,ecx //0048D66C: wrapped DWORD difference
        sar eax,0x2 //0048D66E: retain these flags across following LEAs/PUSH
        lea eax,[eax*0x4] //0048D671: wrapped byte span
        push esi //0048D678
        lea esi,[eax+edx] //0048D679: captured return pointer
        jz copy_forward_done //0048D67C: zero only, negative count still copies
        push eax //0048D67E: actual count bytes
        push ecx //0048D67F: actual source
        push eax //0048D680: actual destination capacity
        push edx //0048D681: actual destination
        call memmove_s //0048D682: genuine selected SDK C API
        add esp,0x10 //0048D687
    copy_forward_done:
        mov eax,esi //0048D68A
        pop esi //0048D68C
        ret 0xc //0048D68D: exactly three actual words
    }
}

__declspec(naked) void* __stdcall fill_n_native_traffic_dwords_0048e4a0(
    void*, std::uint32_t, const void*) {
    __asm {
        push esi //0048E4A0
        mov esi,[esp+0xc] //0048E4A1: DWORD count
        test esi,esi //0048E4A5: CF=0; JBE below means count==0
        push edi //0048E4A7
        mov edi,[esp+0xc] //0048E4A8: captured first
        mov eax,esi //0048E4AC
        mov ecx,edi //0048E4AE
        jbe fill_n_done //0048E4B0: do not read value-cell argument when zero
        mov edx,[esp+0x14] //0048E4B2: capture actual third cell once
        push ebx //0048E4B6
    fill_n_loop:
        mov ebx,[edx] //0048E4B7: reread borrowed value each iteration
        mov [ecx],ebx //0048E4B9
        sub eax,0x1 //0048E4BB: DWORD count, including high-bit input
        add ecx,0x4 //0048E4BE: DWORD pointer wrap
        test eax,eax //0048E4C1
        ja fill_n_loop //0048E4C3: CF=0; unsigned nonzero
        pop ebx //0048E4C5
    fill_n_done:
        lea eax,[edi+esi*0x4] //0048E4C6: captured wrapped result
        pop edi //0048E4C9
        pop esi //0048E4CA
        ret 0xc //0048E4CB
    }
}

} // namespace bsp
