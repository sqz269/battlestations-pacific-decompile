#include "bsp/native_pod8_vector_append.hpp"

#include "bsp/native_invalid_parameter_noinfo_call.hpp"
#include "bsp/native_pod8_range.hpp"
#include "bsp/native_pod8_vector_insert.hpp"

namespace bsp {
static_assert(sizeof(void*) == 4);

__declspec(naked) void* __fastcall insert_one_native_pod8_vector_checked_00421a50(
    void*, void*, void*, const void*, void*, const void*) {
    __asm {
        push ecx
        push ebx
        push ebp
        mov ebp, dword ptr [esp + 14h]
        push esi
        mov esi, ecx
        push edi
        mov edi, dword ptr [esi + 4]
        test edi, edi
        jz empty_index
        mov eax, dword ptr [esi + 8]
        mov ecx, eax
        sub ecx, edi
        sar ecx, 3
        jnz validate_owner
    empty_index:
        xor ebx, ebx
        jmp insert_value
    validate_owner:
        cmp edi, eax
        jbe ordered_first
        call invoke_native_invalid_parameter_00bf6713
    ordered_first:
        test ebp, ebp
        jz invalid_owner
        cmp ebp, esi
        je owner_checked
    invalid_owner:
        call invoke_native_invalid_parameter_00bf6713
    owner_checked:
        mov ebx, dword ptr [esp + 20h]
        sub ebx, edi
        sar ebx, 3
    insert_value:
        mov edx, dword ptr [esp + 24h]
        mov eax, dword ptr [esp + 20h]
        push edx
        push 1
        push eax
        push ebp
        mov ecx, esi
        call insert_native_pod8_vector_00420f80
        mov edi, dword ptr [esi + 4]
        cmp edi, dword ptr [esi + 8]
        jbe ordered_result
        call invoke_native_invalid_parameter_00bf6713
    ordered_result:
        mov dword ptr [esp + 20h], edi
        lea edi, [edi + ebx * 8]
        cmp edi, dword ptr [esi + 8]
        ja invalid_result
        cmp edi, dword ptr [esi + 4]
        jae result_checked
    invalid_result:
        call invoke_native_invalid_parameter_00bf6713
    result_checked:
        mov eax, dword ptr [esp + 18h]
        mov dword ptr [eax + 4], edi
        pop edi
        mov dword ptr [eax], esi
        pop esi
        pop ebp
        pop ebx
        pop ecx
        ret 10h
    }
}

__declspec(naked) void __fastcall append_native_pod8_vector_004223b0(
    void*, void*, const void*) {
    __asm {
        sub esp, 8
        push esi
        mov esi, ecx
        mov edx, dword ptr [esi + 4]
        test edx, edx
        push edi
        jnz have_first
        xor ecx, ecx
        jmp size_ready
    have_first:
        mov ecx, dword ptr [esi + 8]
        sub ecx, edx
        sar ecx, 3
    size_ready:
        test edx, edx
        jz grow
        mov eax, dword ptr [esi + 0ch]
        sub eax, edx
        sar eax, 3
        cmp ecx, eax
        jae grow
        mov ecx, dword ptr [esp + 14h]
        mov edx, dword ptr [esp + 14h]
        mov edi, dword ptr [esi + 8]
        mov byte ptr [esp + 8], 0
        mov eax, dword ptr [esp + 8]
        push eax
        push ecx
        push esi
        push edx
        mov edx, 1
        mov ecx, edi
        call fill_n_native_pod8_range_0041c7e0
        add edi, 8
        mov dword ptr [esi + 8], edi
        pop edi
        pop esi
        add esp, 8
        ret 4
    grow:
        mov edi, dword ptr [esi + 8]
        cmp edx, edi
        jbe ordered_growth
        call invoke_native_invalid_parameter_00bf6713
    ordered_growth:
        mov eax, dword ptr [esp + 14h]
        push eax
        push edi
        push esi
        lea ecx, [esp + 14h]
        push ecx
        mov ecx, esi
        call insert_one_native_pod8_vector_checked_00421a50
        pop edi
        pop esi
        add esp, 8
        ret 4
    }
}

} // namespace bsp
