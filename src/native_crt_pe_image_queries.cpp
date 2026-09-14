#include "bsp/native_crt_pe_image_queries.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native PE image query entries require MSVC Win32.
#endif

namespace bsp {
// Preserve the original register effects, read order and short branch layout.
__declspec(naked) std::int32_t __cdecl
validate_native_crt_image_base_00c16d50(const void*) {
    __asm {
        mov ecx, dword ptr [esp + 4]
        cmp word ptr [ecx], 05a4dh
        je validate_nt
    validate_false:
        xor eax, eax
        ret
    validate_nt:
        mov eax, dword ptr [ecx + 03ch]
        add eax, ecx
        cmp dword ptr [eax], 04550h
        jne validate_false
        xor ecx, ecx
        cmp word ptr [eax + 018h], 010bh
        sete cl
        mov eax, ecx
        ret
    }
}

__declspec(naked) const void* __cdecl
find_native_crt_pe_section_00c16d80(const void*, std::uint32_t) {
    __asm {
        mov eax, dword ptr [esp + 4]
        mov ecx, dword ptr [eax + 03ch]
        add ecx, eax
        movzx eax, word ptr [ecx + 014h]
        push ebx
        push esi
        movzx esi, word ptr [ecx + 6]
        xor edx, edx
        test esi, esi
        push edi
        lea eax, [eax + ecx + 018h]
        jbe section_missing
        mov edi, dword ptr [esp + 014h]
    section_next:
        mov ecx, dword ptr [eax + 0ch]
        cmp edi, ecx
        jb section_advance
        mov ebx, dword ptr [eax + 8]
        add ebx, ecx
        cmp edi, ebx
        jb section_return
    section_advance:
        add edx, 1
        add eax, 028h
        cmp edx, esi
        jb section_next
    section_missing:
        xor eax, eax
    section_return:
        pop edi
        pop esi
        pop ebx
        ret
    }
}
} // namespace bsp
