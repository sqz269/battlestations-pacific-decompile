#include "bsp/native_singleton_vector_registration_wrappers.hpp"

#include "bsp/native_singleton_vector_insert_count.hpp"

#include <cstdlib>

namespace bsp {
namespace {

// Fixed actual SDK service. The no-argument source call has the same five-byte
// CALL site width as BF6713; its current CRT owns the handler and fatal path.
__declspec(noinline) void __cdecl current_crt_invalid_parameter() {
    _invalid_parameter_noinfo();
}

} // namespace

__declspec(naked) void* __fastcall insert_one_native_singleton_slots_checked_00bd08d0(
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
        jz zero_index
        mov eax, dword ptr [esi + 8]
        mov ecx, eax
        sub ecx, edi
        sar ecx, 2
        jnz nonempty_index
    zero_index:
        xor ebx, ebx
        jmp perform_insert
    nonempty_index:
        cmp edi, eax
        jbe validate_iterator_owner
        call current_crt_invalid_parameter
    validate_iterator_owner:
        test ebp, ebp
        jz invalid_iterator_owner
        cmp ebp, esi
        je compute_index
    invalid_iterator_owner:
        call current_crt_invalid_parameter
    compute_index:
        mov ebx, dword ptr [esp + 20h]
        sub ebx, edi
        sar ebx, 2
    perform_insert:
        mov edx, dword ptr [esp + 24h]
        mov eax, dword ptr [esp + 20h]
        push edx
        push 1
        push eax
        push ebp
        mov ecx, esi
        call insert_count_native_singleton_slots_00bd0700
        mov edi, dword ptr [esi + 4]
        cmp edi, dword ptr [esi + 8]
        jbe derive_result
        call current_crt_invalid_parameter
    derive_result:
        mov dword ptr [esp + 20h], edi
        lea edi, [edi + ebx * 4]
        cmp edi, dword ptr [esi + 8]
        ja invalid_result
        cmp edi, dword ptr [esi + 4]
        jae write_result
    invalid_result:
        call current_crt_invalid_parameter
    write_result:
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

__declspec(naked) void __fastcall append_native_singleton_slot_00bd0bc0(
    void*, void*, const void*) {
    __asm {
        sub esp, 8
        push esi
        mov esi, ecx
        mov edx, dword ptr [esi + 4]
        test edx, edx
        jnz captured_begin_nonnull
        xor ecx, ecx
        jmp compare_capacity
    captured_begin_nonnull:
        mov ecx, dword ptr [esi + 8]
        sub ecx, edx
        sar ecx, 2
    compare_capacity:
        test edx, edx
        jz insert_with_growth
        mov eax, dword ptr [esi + 0Ch]
        sub eax, edx
        sar eax, 2
        cmp ecx, eax
        jae insert_with_growth
        mov eax, dword ptr [esi + 8]
        mov ecx, dword ptr [esp + 10h]
        mov edx, dword ptr [ecx]
        mov dword ptr [eax], edx
        add eax, 4
        mov dword ptr [esi + 8], eax
        pop esi
        add esp, 8
        ret 4
    insert_with_growth:
        push edi
        mov edi, dword ptr [esi + 8]
        cmp edx, edi
        jbe call_checked_insert
        call current_crt_invalid_parameter
    call_checked_insert:
        mov eax, dword ptr [esp + 14h]
        push eax
        push edi
        push esi
        lea ecx, [esp + 14h]
        push ecx
        mov ecx, esi
        call insert_one_native_singleton_slots_checked_00bd08d0
        pop edi
        pop esi
        add esp, 8
        ret 4
    }
}

__declspec(naked) void __fastcall register_native_singleton_object_00bd0c30(
    void*, void*, void*) {
    __asm {
        push esi
        mov esi, ecx
        mov eax, dword ptr [esi + 8]
        cmp dword ptr [esi + 4], eax
        jbe test_current_object
        call current_crt_invalid_parameter
    test_current_object:
        cmp dword ptr [esp + 8], 0
        je finished
        lea eax, [esp + 8]
        push eax
        mov ecx, esi
        call append_native_singleton_slot_00bd0bc0
    finished:
        pop esi
        ret 4
    }
}

} // namespace bsp
