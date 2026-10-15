#include "bsp/native_avoid_clip_cleanup.hpp"

#include "bsp/singleton_lifetime.hpp"

namespace bsp {

static_assert(sizeof(void*) == 4);

__declspec(naked) void __fastcall clear_native_avoid_corner_records_004167c0(void*) {
    __asm {
        push esi
        mov esi, ecx
        mov eax, dword ptr [esi + 4]
        push edi
        xor edi, edi
        test eax, eax
        jbe no_records
        // Exact three-byte LEA ECX,[ECX+0] at004167CD.
        _emit 08dh
        _emit 049h
        _emit 000h
    next_record:
        mov ecx, dword ptr [esi]
        mov edx, dword ptr [ecx + edi * 4]
        push edx
        call singleton_lifetime_free
        mov eax, dword ptr [esi + 4]
        add edi, 1
        add esp, 4
        cmp edi, eax
        jb next_record
        pop edi
        mov dword ptr [esi + 4], 0
        pop esi
        ret
    no_records:
        mov dword ptr [esi + 4], edi
        pop edi
        pop esi
        ret
    }
}

__declspec(naked) void __fastcall free_native_current_pointer_cell_00412ef0(void*) {
    __asm {
        mov eax, dword ptr [ecx]
        test eax, eax
        je finished
        push eax
        call singleton_lifetime_free
        pop ecx
    finished:
        ret
    }
}

__declspec(naked) void __fastcall free_and_clear_native_avoid_pointer_00412dc0(void*) {
    __asm {
        push esi
        mov esi, ecx
        mov eax, dword ptr [esi]
        test eax, eax
        je finished
        push eax
        call singleton_lifetime_free
        add esp, 4
        mov dword ptr [esi], 0
    finished:
        pop esi
        ret
    }
}

} // namespace bsp
