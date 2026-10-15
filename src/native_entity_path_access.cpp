#include "bsp/native_entity_path_access.hpp"

namespace bsp {

static_assert(sizeof(void*) == 4);
static_assert(sizeof(std::int32_t) == 4);

__declspec(naked) void* __fastcall get_native_entity_path_interface_007ac9d0(
    void*, void*) {
    __asm {
        push esi
        mov esi, ecx
        test esi, esi
        jz no_interface
        mov eax, dword ptr [esi]
        mov edx, dword ptr [eax + 5ch]
        push 47h
        call edx
        test al, al
        jz try_kind48
    common_interface:
        lea eax, [esi + 1e4h]
        pop esi
        ret
    try_kind48:
        mov eax, dword ptr [esi]
        mov edx, dword ptr [eax + 5ch]
        push 48h
        mov ecx, esi
        call edx
        test al, al
        jz try_kind49
        lea eax, [esi + 170h]
        pop esi
        ret
    try_kind49:
        mov eax, dword ptr [esi]
        mov edx, dword ptr [eax + 5ch]
        push 49h
        mov ecx, esi
        call edx
        test al, al
        jz try_kind4a
        lea eax, [esi + 310h]
        pop esi
        ret
    try_kind4a:
        mov eax, dword ptr [esi]
        mov edx, dword ptr [eax + 5ch]
        push 4ah
        mov ecx, esi
        call edx
        test al, al
        jnz common_interface
    no_interface:
        xor eax, eax
        pop esi
        ret
    }
}

__declspec(naked) void* __fastcall get_native_entity_ancestor_00923810(
    void*, void*, std::int32_t) {
    __asm {
        mov eax, dword ptr [ecx + 3ch]
        mov ecx, dword ptr [esp + 4]
        sub ecx, 1
        test eax, eax
        jz done
        // Preserve the Native two-byte identity instruction at 0092381E.
        _emit 8bh
        _emit 0ffh
    next_parent:
        test ecx, ecx
        jle done
        mov eax, dword ptr [eax + 3ch]
        sub ecx, 1
        test eax, eax
        jnz next_parent
    done:
        ret 4
    }
}

} // namespace bsp
