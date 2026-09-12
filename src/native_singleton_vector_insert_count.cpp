#include "bsp/native_singleton_vector_insert_count.hpp"

#include "bsp/native_singleton_vector_allocation.hpp"
#include "bsp/native_singleton_vector_leaves.hpp"
#include "bsp/singleton_lifetime.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native singleton vector count insertion requires MSVC Win32.
#endif

namespace bsp {
static_assert(sizeof(void*) == 4 && sizeof(std::uint32_t) == 4);

// Original raw instruction schedule preserves retained registers, current
// field reloads and aliases to argument storage. Native BD0801's returning
// free stack adjustment is included despite the stale saved flow gap.
__declspec(naked) void __fastcall insert_count_native_singleton_slots_00bd0700(
    void*, void*, const void*, void*, std::uint32_t, const void*) {
    __asm {
        mov eax, dword ptr [esp + 010h]
        push ebx
        push esi
        mov esi, ecx
        mov ecx, dword ptr [eax]
        mov dword ptr [esp + 018h], ecx
        mov ecx, dword ptr [esi + 4]
        test ecx, ecx
        push edi
        jne at_00bd071a
        xor edi, edi
        jmp at_00bd0722
    at_00bd071a:
        mov edi, dword ptr [esi + 0ch]
        sub edi, ecx
        sar edi, 2
    at_00bd0722:
        mov ebx, dword ptr [esp + 018h]
        test ebx, ebx
        je at_00bd08ae
        test ecx, ecx
        jne at_00bd0736
        xor eax, eax
        jmp at_00bd073e
    at_00bd0736:
        mov eax, dword ptr [esi + 8]
        sub eax, ecx
        sar eax, 2
    at_00bd073e:
        mov edx, 03fffffffh
        sub edx, eax
        cmp edx, ebx
        jae at_00bd074e
        call native_singleton_length_error_00bd0590
    at_00bd074e:
        test ecx, ecx
        jne at_00bd0756
        xor eax, eax
        jmp at_00bd075e
    at_00bd0756:
        mov eax, dword ptr [esi + 8]
        sub eax, ecx
        sar eax, 2
    at_00bd075e:
        add eax, ebx
        cmp edi, eax
        push ebp
        jae at_00bd081c
        mov eax, edi
        shr eax, 1
        mov edx, 03fffffffh
        sub edx, eax
        cmp edx, edi
        jae at_00bd077c
        xor edi, edi
        jmp at_00bd077e
    at_00bd077c:
        add edi, eax
    at_00bd077e:
        test ecx, ecx
        jne at_00bd0786
        xor eax, eax
        jmp at_00bd078e
    at_00bd0786:
        mov eax, dword ptr [esi + 8]
        sub eax, ecx
        sar eax, 2
    at_00bd078e:
        add eax, ebx
        cmp edi, eax
        jae at_00bd07a7
        test ecx, ecx
        jne at_00bd079c
        xor eax, eax
        jmp at_00bd07a4
    at_00bd079c:
        mov eax, dword ptr [esi + 8]
        sub eax, ecx
        sar eax, 2
    at_00bd07a4:
        lea edi, [eax + ebx]
    at_00bd07a7:
        xor edx, edx
        mov ecx, edi
        call native_singleton_pointer_allocate_00bcfeb0
        mov ecx, dword ptr [esi + 4]
        mov ebp, eax
        mov eax, dword ptr [esp + 018h]
        push ebp
        push eax
        push ecx
        mov ecx, esi
        call copy_native_singleton_slots_00bd0500
        lea edx, [esp + 020h]
        push edx
        push ebx
        push eax
        mov ecx, esi
        call fill_native_singleton_slots_00bd0560
        mov ecx, dword ptr [esp + 018h]
        push eax
        mov eax, dword ptr [esi + 8]
        push eax
        push ecx
        mov ecx, esi
        call copy_native_singleton_slots_00bd0500
        mov eax, dword ptr [esi + 4]
        test eax, eax
        jne at_00bd07ed
        xor ecx, ecx
        jmp at_00bd07f5
    at_00bd07ed:
        mov ecx, dword ptr [esi + 8]
        sub ecx, eax
        sar ecx, 2
    at_00bd07f5:
        add ebx, ecx
        test eax, eax
        je at_00bd0804
        push eax
        call singleton_lifetime_free
        add esp, 4
    at_00bd0804:
        lea edx, [ebp + edi*4]
        lea eax, [ebp + ebx*4]
        mov dword ptr [esi + 4], ebp
        pop ebp
        pop edi
        mov dword ptr [esi + 0ch], edx
        mov dword ptr [esi + 8], eax
        pop esi
        pop ebx
        ret 010h
    at_00bd081c:
        mov ebp, dword ptr [esi + 8]
        mov edi, dword ptr [esp + 018h]
        mov ecx, ebp
        sub ecx, edi
        sar ecx, 2
        lea eax, [ebx*4]
        cmp ecx, ebx
        mov dword ptr [esp + 01ch], eax
        mov ecx, esi
        jae at_00bd0881
        add eax, edi
        push eax
        push ebp
        push edi
        call copy_native_singleton_slots_00bd0500
        mov eax, dword ptr [esi + 8]
        mov ecx, eax
        sub ecx, edi
        sar ecx, 2
        lea edx, [esp + 020h]
        push edx
        sub ebx, ecx
        push ebx
        push eax
        mov ecx, esi
        call fill_native_singleton_slots_00bd0560
        mov eax, dword ptr [esp + 01ch]
        add dword ptr [esi + 8], eax
        mov esi, dword ptr [esi + 8]
        lea edx, [esp + 020h]
        push edx
        sub esi, eax
        push esi
        push edi
        call assign_native_singleton_slots_00bd0160
        add esp, 0ch
        pop ebp
        pop edi
        pop esi
        pop ebx
        ret 010h
    at_00bd0881:
        push ebp
        mov ebx, ebp
        sub ebx, eax
        push ebp
        push ebx
        call copy_native_singleton_slots_00bd0500
        push ebp
        push ebx
        push edi
        mov dword ptr [esi + 8], eax
        call copy_backward_native_singleton_slots_00bd0180
        mov ecx, dword ptr [esp + 028h]
        lea eax, [esp + 02ch]
        push eax
        add ecx, edi
        push ecx
        push edi
        call assign_native_singleton_slots_00bd0160
        add esp, 018h
        pop ebp
    at_00bd08ae:
        pop edi
        pop esi
        pop ebx
        ret 010h
    }
}
} // namespace bsp
