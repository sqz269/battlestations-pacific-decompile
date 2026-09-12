#include "bsp/native_singleton_removal_reorder.hpp"
#include "bsp/native_singleton_vector_leaves.hpp"
#include "bsp/native_singleton_vector_registration_wrappers.hpp"

#include <cstdlib>
#include <cstring>

namespace bsp {
namespace {
// Fixed actual CRT handler service; returning handlers remain possible.
__declspec(noinline) void __cdecl current_crt_invalid_parameter() {
    _invalid_parameter_noinfo();
}
} // namespace

// Complete original raw instruction schedule, including redundant validations.
__declspec(naked) void __fastcall unregister_native_singleton_object_00bcfca0(void*, void*, void*) {
    __asm {
        push ebx
        mov ebx, dword ptr [esp + 8]
        test ebx, ebx
        push esi
        mov esi, ecx
        je at_00bcfd0e
        push edi
        xor edi, edi
        call count_native_singleton_slots_00bcf910
        test eax, eax
        jbe at_00bcfd0d
    at_00bcfcb8:
        mov ecx, dword ptr [esi + 4]
        test ecx, ecx
        je at_00bcfccb
        mov eax, dword ptr [esi + 8]
        sub eax, ecx
        sar eax, 2
        cmp edi, eax
        jb at_00bcfcd0
    at_00bcfccb:
        call current_crt_invalid_parameter
    at_00bcfcd0:
        mov eax, dword ptr [esi + 4]
        cmp dword ptr [eax + edi*4], ebx
        je at_00bcfcec
        mov ecx, esi
        add edi, 1
        call count_native_singleton_slots_00bcf910
        cmp edi, eax
        jb at_00bcfcb8
        pop edi
        pop esi
        pop ebx
        ret 4
    at_00bcfcec:
        mov ecx, eax
        test ecx, ecx
        je at_00bcfcfe
        mov eax, dword ptr [esi + 8]
        sub eax, ecx
        sar eax, 2
        cmp edi, eax
        jb at_00bcfd03
    at_00bcfcfe:
        call current_crt_invalid_parameter
    at_00bcfd03:
        mov ecx, dword ptr [esi + 4]
        mov dword ptr [ecx + edi*4], 0
    at_00bcfd0d:
        pop edi
    at_00bcfd0e:
        pop esi
        pop ebx
        ret 4
    }
}

// Complete original raw instruction schedule, including redundant validations.
__declspec(naked) void __fastcall move_native_singleton_object_after_00bd0d70(void*, void*, void*, void*) {
    __asm {
        sub esp, 8
        push ebx
        push ebp
        push esi
        mov esi, ecx
        push edi
        mov edi, dword ptr [esi + 4]
        cmp edi, dword ptr [esi + 8]
        jbe at_00bd0d86
        call current_crt_invalid_parameter
    at_00bd0d86:
        mov ebp, dword ptr [esp + 0x1c]
        // Original six-byte alignment LEA EBX,[EBX+0].
        _emit 08dh
        _emit 09bh
        _emit 000h
        _emit 000h
        _emit 000h
        _emit 000h
    at_00bd0d90:
        mov ebx, dword ptr [esi + 8]
        cmp dword ptr [esi + 4], ebx
        jbe at_00bd0d9d
        call current_crt_invalid_parameter
    at_00bd0d9d:
        test esi, esi
        je at_00bd0da5
        cmp esi, esi
        je at_00bd0daa
    at_00bd0da5:
        call current_crt_invalid_parameter
    at_00bd0daa:
        cmp edi, ebx
        je at_00bd0dd4
        test esi, esi
        jne at_00bd0db7
        call current_crt_invalid_parameter
    at_00bd0db7:
        cmp edi, dword ptr [esi + 8]
        jb at_00bd0dc1
        call current_crt_invalid_parameter
    at_00bd0dc1:
        cmp dword ptr [edi], ebp
        je at_00bd0dd4
        cmp edi, dword ptr [esi + 8]
        jb at_00bd0dcf
        call current_crt_invalid_parameter
    at_00bd0dcf:
        add edi, 4
        jmp at_00bd0d90
    at_00bd0dd4:
        test esi, esi
        jne at_00bd0ddd
        call current_crt_invalid_parameter
    at_00bd0ddd:
        cmp edi, dword ptr [esi + 8]
        jb at_00bd0de7
        call current_crt_invalid_parameter
    at_00bd0de7:
        mov eax, dword ptr [edi]
        mov dword ptr [esp + 0x1c], eax
        mov eax, dword ptr [esi + 8]
        lea ecx, [edi + 4]
        sub eax, ecx
        sar eax, 2
        test eax, eax
        jle at_00bd0e0c
        add eax, eax
        add eax, eax
        push eax
        push ecx
        push eax
        push edi
        call memmove_s
        add esp, 0x10
    at_00bd0e0c:
        add dword ptr [esi + 8], -4
        mov eax, dword ptr [esi + 8]
        cmp dword ptr [esi + 4], eax
        jbe at_00bd0e1d
        call current_crt_invalid_parameter
    at_00bd0e1d:
        mov edi, dword ptr [esi + 4]
        cmp edi, dword ptr [esi + 8]
        jbe at_00bd0e2a
        call current_crt_invalid_parameter
    at_00bd0e2a:
        mov ebp, dword ptr [esp + 0x20]
        mov edi, edi
    at_00bd0e30:
        mov ebx, dword ptr [esi + 8]
        cmp dword ptr [esi + 4], ebx
        jbe at_00bd0e3d
        call current_crt_invalid_parameter
    at_00bd0e3d:
        test esi, esi
        je at_00bd0e45
        cmp esi, esi
        je at_00bd0e4a
    at_00bd0e45:
        call current_crt_invalid_parameter
    at_00bd0e4a:
        cmp edi, ebx
        je at_00bd0e74
        test esi, esi
        jne at_00bd0e57
        call current_crt_invalid_parameter
    at_00bd0e57:
        cmp edi, dword ptr [esi + 8]
        jb at_00bd0e61
        call current_crt_invalid_parameter
    at_00bd0e61:
        cmp dword ptr [edi], ebp
        je at_00bd0e74
        cmp edi, dword ptr [esi + 8]
        jb at_00bd0e6f
        call current_crt_invalid_parameter
    at_00bd0e6f:
        add edi, 4
        jmp at_00bd0e30
    at_00bd0e74:
        mov eax, dword ptr [esi + 8]
        cmp dword ptr [esi + 4], eax
        mov dword ptr [esp + 0x20], eax
        jbe at_00bd0e85
        call current_crt_invalid_parameter
    at_00bd0e85:
        test esi, esi
        mov ebp, esi
        mov dword ptr [esp + 0x14], edi
        jne at_00bd0e94
        call current_crt_invalid_parameter
    at_00bd0e94:
        lea ebx, [edi + 4]
        cmp ebx, dword ptr [esi + 8]
        ja at_00bd0ea1
        cmp ebx, dword ptr [esi + 4]
        jae at_00bd0ea6
    at_00bd0ea1:
        call current_crt_invalid_parameter
    at_00bd0ea6:
        cmp ebp, esi
        je at_00bd0eaf
        call current_crt_invalid_parameter
    at_00bd0eaf:
        cmp ebx, dword ptr [esp + 0x20]
        jne at_00bd0f1a
        mov edx, dword ptr [esi + 4]
        test edx, edx
        jne at_00bd0ec0
        xor ecx, ecx
        jmp at_00bd0ec8
    at_00bd0ec0:
        mov ecx, dword ptr [esi + 8]
        sub ecx, edx
        sar ecx, 2
    at_00bd0ec8:
        test edx, edx
        je at_00bd0ef1
        mov eax, dword ptr [esi + 0xc]
        sub eax, edx
        sar eax, 2
        cmp ecx, eax
        jae at_00bd0ef1
        mov eax, dword ptr [esi + 8]
        mov ecx, dword ptr [esp + 0x1c]
        mov dword ptr [eax], ecx
        pop edi
        add eax, 4
        mov dword ptr [esi + 8], eax
        pop esi
        pop ebp
        pop ebx
        add esp, 8
        ret 8
    at_00bd0ef1:
        mov edi, dword ptr [esi + 8]
        cmp edx, edi
        jbe at_00bd0efd
        call current_crt_invalid_parameter
    at_00bd0efd:
        lea edx, [esp + 0x1c]
        push edx
        push edi
        push esi
        lea eax, [esp + 0x1c]
        push eax
        mov ecx, esi
        call insert_one_native_singleton_slots_checked_00bd08d0
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 8
        ret 8
    at_00bd0f1a:
        cmp ebx, dword ptr [esi + 8]
        mov dword ptr [esp + 0x14], edi
        ja at_00bd0f28
        cmp ebx, dword ptr [esi + 4]
        jae at_00bd0f2d
    at_00bd0f28:
        call current_crt_invalid_parameter
    at_00bd0f2d:
        lea ecx, [esp + 0x1c]
        push ecx
        push ebx
        push ebp
        lea edx, [esp + 0x1c]
        push edx
        mov ecx, esi
        call insert_one_native_singleton_slots_checked_00bd08d0
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 8
        ret 8
    }
}

} // namespace bsp
