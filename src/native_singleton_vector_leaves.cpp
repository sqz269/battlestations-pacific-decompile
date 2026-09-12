#include "bsp/native_singleton_vector_leaves.hpp"

#include "bsp/singleton_lifetime.hpp"

#include <cstring>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native singleton vector leaves require MSVC Win32.
#endif

namespace bsp {
static_assert(sizeof(void*) == 4 && sizeof(std::uint32_t) == 4);

__declspec(naked) std::uint32_t __fastcall count_native_singleton_slots_00bcf910(
    const void*, void*) noexcept {
    __asm {
        mov edx, dword ptr [ecx + 4]
        test edx, edx
        jne nonempty_storage
        xor eax, eax
        ret
    nonempty_storage:
        mov eax, dword ptr [ecx + 8]
        sub eax, edx
        sar eax, 2
        ret
    }
}

__declspec(naked) void __cdecl assign_native_singleton_slots_00bd0160(
    void*, const void*, const void*) noexcept {
    __asm {
        mov eax, dword ptr [esp + 4]
        mov ecx, dword ptr [esp + 8]
        cmp eax, ecx
        je finished
        mov edx, dword ptr [esp + 0Ch]
        push esi
    next_slot:
        mov esi, dword ptr [edx]
        mov dword ptr [eax], esi
        add eax, 4
        cmp eax, ecx
        jne next_slot
        pop esi
    finished:
        ret
    }
}

__declspec(naked) void* __cdecl copy_backward_native_singleton_slots_00bd0180(
    const void*, const void*, void*) {
    __asm {
        mov eax, dword ptr [esp + 8]
        mov edx, dword ptr [esp + 4]
        sub eax, edx
        sar eax, 2
        push esi
        mov esi, dword ptr [esp + 10h]
        lea ecx, [eax * 4]
        sub esi, ecx
        test eax, eax
        jle finished
        push ecx
        push edx
        push ecx
        push esi
        call memmove_s
        add esp, 10h
    finished:
        mov eax, esi
        pop esi
        ret
    }
}

__declspec(naked) void __fastcall clear_native_singleton_storage_00bd0220(
    void*, void*) noexcept {
    __asm {
        push esi
        mov esi, ecx
        mov eax, dword ptr [esi + 4]
        test eax, eax
        je clear_fields
        push eax
        call singleton_lifetime_free
        add esp, 4
    clear_fields:
        mov dword ptr [esi + 4], 0
        mov dword ptr [esi + 8], 0
        mov dword ptr [esi + 0Ch], 0
        pop esi
        ret
    }
}

__declspec(naked) void* __stdcall copy_native_singleton_slots_00bd0500(
    const void*, const void*, void*) {
    __asm {
        mov eax, dword ptr [esp + 8]
        mov ecx, dword ptr [esp + 4]
        mov edx, dword ptr [esp + 0Ch]
        sub eax, ecx
        sar eax, 2
        lea eax, [eax * 4]
        push esi
        lea esi, [eax + edx]
        // This JE consumes the SAR flags; the intervening LEAs preserve them.
        je finished
        push eax
        push ecx
        push eax
        push edx
        call memmove_s
        add esp, 10h
    finished:
        mov eax, esi
        pop esi
        ret 0Ch
    }
}

__declspec(naked) void* __stdcall fill_native_singleton_slots_00bd0560(
    void*, std::uint32_t, const void*) noexcept {
    __asm {
        push esi
        mov esi, dword ptr [esp + 0Ch]
        test esi, esi
        push edi
        mov edi, dword ptr [esp + 0Ch]
        mov eax, esi
        mov ecx, edi
        jbe finished
        mov edx, dword ptr [esp + 14h]
        push ebx
    next_slot:
        mov ebx, dword ptr [edx]
        mov dword ptr [ecx], ebx
        sub eax, 1
        add ecx, 4
        test eax, eax
        ja next_slot
        pop ebx
    finished:
        lea eax, [edi + esi * 4]
        pop edi
        pop esi
        ret 0Ch
    }
}
} // namespace bsp
