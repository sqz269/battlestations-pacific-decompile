#include "bsp/native_material_texture_source_constants.hpp"

namespace bsp {
static_assert(sizeof(void*) == 4);
__declspec(naked) void* __fastcall native_texture_source_current_texture_00c302f0(const void*) noexcept {
    __asm {
        cmp dword ptr [ecx+14h], 0
        je empty
        mov eax, [ecx+8]
        mov ecx, [ecx+10h]
        mov eax, [ecx+eax*4]
        ret
    empty:
        xor eax, eax
        ret
    }
}
__declspec(naked) void __fastcall native_caustics_source_constants_00bbcc40(
    const void*, void*, void*, const void*, const void*, void*, void*) noexcept {
    __asm {
        mov edx, [esp+8]
        mov al, [edx+1eh]
        cmp al, 0ffh
        push esi
        mov esi, [esp+14h]
        je color
        movss xmm0, [ecx+30h]
        movzx eax, al
        shl eax, 4
        lea eax, [eax+esi+4]
        movaps xmm1, xmm0
        movss [eax+4], xmm0
        xorps xmm0, xmm0
        movss [eax], xmm1
        movss [eax+8], xmm0
        movss [eax+0ch], xmm0
    color:
        mov dl, [edx+1fh]
        cmp dl, 0ffh
        je done
        lea eax, [ecx+20h]
        movzx ecx, dl
        mov edx, [eax]
        shl ecx, 4
        lea ecx, [ecx+esi+4]
        mov [ecx], edx
        mov edx, [eax+4]
        mov [ecx+4], edx
        mov edx, [eax+8]
        mov [ecx+8], edx
        mov eax, [eax+0ch]
        mov [ecx+0ch], eax
    done:
        pop esi
        ret 14h
    }
}
__declspec(naked) void __fastcall native_shore_source_constants_00bbcbd0(
    const void*, void*, void*, const void*, const void*, void*, void*) noexcept {
    __asm { ret 14h }
}
} // namespace bsp
