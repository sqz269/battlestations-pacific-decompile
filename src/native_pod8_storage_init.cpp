#include "bsp/native_pod8_storage_init.hpp"

#include "bsp/native_invalid_parameter_noinfo_call.hpp"
#include "bsp/native_pod8_failure.hpp"

namespace bsp {

static_assert(sizeof(void*) == 4 && sizeof(bool) == 1);

__declspec(naked) void* __stdcall allocate_native_pod8_storage_00418700(std::uint32_t) {
    __asm {
        mov ecx, dword ptr [esp + 4]
        xor edx, edx
        call allocate_native_pod8_records_00413390
        ret 4
    }
}

__declspec(naked) bool __fastcall initialize_native_pod8_storage_00420ac0(
    void*, void*, std::uint32_t) {
    __asm {
        push esi
        xor eax, eax
        push edi
        mov edi, dword ptr [esp + 0ch]
        cmp edi, eax
        mov esi, ecx
        mov dword ptr [esi + 4], eax
        mov dword ptr [esi + 8], eax
        mov dword ptr [esi + 0ch], eax
        jne nonzero_count
        pop edi
        xor al, al
        pop esi
        ret 4
    nonzero_count:
        cmp edi, 1fffffffh
        jbe allocate_storage
        call throw_native_pod8_vector_length_error_004203a0
    allocate_storage:
        xor edx, edx
        mov ecx, edi
        call allocate_native_pod8_records_00413390
        mov dword ptr [esi + 4], eax
        mov dword ptr [esi + 8], eax
        lea eax, [eax + edi * 8]
        mov dword ptr [esi + 0ch], eax
        pop edi
        mov al, 1
        pop esi
        ret 4
    }
}

__declspec(naked) std::int32_t __fastcall difference_native_pod8_iterator_00418710(
    const void*, void*, const void*) {
    __asm {
        push esi
        mov esi, ecx
        mov eax, dword ptr [esi]
        test eax, eax
        push edi
        mov edi, dword ptr [esp + 0ch]
        je invalid_owner
        cmp eax, dword ptr [edi]
        je owners_match
    invalid_owner:
        call invoke_native_invalid_parameter_00bf6713
    owners_match:
        mov eax, dword ptr [esi + 4]
        sub eax, dword ptr [edi + 4]
        pop edi
        sar eax, 3
        pop esi
        ret 4
    }
}

} // namespace bsp
