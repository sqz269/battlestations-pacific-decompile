#include "bsp/native_crt_eh_validation.hpp"
#include "bsp/native_crt_pe_image_queries.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native EH validation requires MSVC Win32.
#endif

namespace bsp {
// ECX and EDI are inherited raw inputs. Only the original stack word appears
// in this declaration; do not synthesize register bindings or a host callback.
__declspec(naked) std::int32_t __cdecl
validate_native_crt_scope_handlers_00c168a0(const void*) {
    __asm {
        push ebx
        push ebp
        mov ebp, dword ptr [esp + 0ch]
        xor eax, eax
        or edx, 0ffffffffh
        cmp ecx, -1
        push esi
        jz scope_success
    scope_next:
        lea ecx, [ecx + ecx * 2]
        mov esi, dword ptr [ebp + ecx * 4 + 8]
        lea ebx, [ebp + ecx * 4]
        sub esi, edi
        and esi, 0fffff000h
        cmp esi, edx
        jz scope_filter
        test eax, eax
        jz handler_find_section
        mov ecx, dword ptr [eax + 0ch]
        cmp esi, ecx
        jc handler_find_section
        mov edx, dword ptr [eax + 8]
        add edx, ecx
        cmp esi, edx
        jc handler_cache_page
    handler_find_section:
        push esi
        push edi
        call find_native_crt_pe_section_00c16d80
        add esp, 8
        test eax, eax
        jz scope_failure
        test dword ptr [eax + 024h], 020000000h
        jz scope_failure
    handler_cache_page:
        mov edx, esi
    scope_filter:
        mov ecx, dword ptr [ebx + 4]
        test ecx, ecx
        jz scope_parent
        sub ecx, edi
        and ecx, 0fffff000h
        mov esi, ecx
        cmp esi, edx
        jz scope_parent
        mov ecx, dword ptr [eax + 0ch]
        cmp esi, ecx
        jc filter_find_section
        mov edx, dword ptr [eax + 8]
        add edx, ecx
        cmp esi, edx
        jc filter_cache_page
    filter_find_section:
        push esi
        push edi
        call find_native_crt_pe_section_00c16d80
        add esp, 8
        test eax, eax
        jz scope_failure
        test dword ptr [eax + 024h], 020000000h
        jz scope_failure
    filter_cache_page:
        mov edx, esi
    scope_parent:
        mov ecx, dword ptr [ebx]
        cmp ecx, -1
        jnz scope_next
    scope_success:
        pop esi
        pop ebp
        mov eax, 1
        pop ebx
        ret
    scope_failure:
        pop esi
        pop ebp
        xor eax, eax
        pop ebx
        ret
    }
}
} // namespace bsp
