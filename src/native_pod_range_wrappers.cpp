#include "bsp/native_pod_range_wrappers.hpp"

#include "bsp/native_game_contact_reports.hpp"

namespace bsp {
__declspec(naked) void* __cdecl vector12_move_forward_dispatch_0041c4d0(
    const void*, const void*, void*) noexcept {
    __asm {
        push ecx // 0041C4D0: preserve the actual tag-word preimage.
        mov ecx, dword ptr [esp + 0ch] // 0041C4D1.
        mov edx, dword ptr [esp + 0ch] // 0041C4D5.
        mov byte ptr [esp], 0 // 0041C4D9: only the tag low byte.
        mov eax, dword ptr [esp] // 0041C4DD.
        push eax // 0041C4E0: metadata tag, ignored by the kernel.
        mov eax, dword ptr [esp + 14h] // 0041C4E1: destination.
        push ecx // 0041C4E5: last.
        mov ecx, dword ptr [esp + 14h] // 0041C4E6: last.
        push edx // 0041C4EA: last.
        mov edx, dword ptr [esp + 14h] // 0041C4EB: first.
        push eax // 0041C4EF: destination.
        push ecx // 0041C4F0: last.
        push edx // 0041C4F1: first.
        call move_native_vector12_00419330 // 0041C4F2: actual existing kernel.
        add esp, 1ch // 0041C4F7: six args plus local tag; EAX unchanged.
        ret // 0041C4FA: caller removes its three arguments.
    }
}

__declspec(naked) void* __cdecl pod8_range_fill_0041c580(
    void*, const void*, const void*) noexcept {
    __asm {
        mov eax, dword ptr [esp + 4] // 0041C580: first/final cursor.
        mov edx, dword ptr [esp + 8] // 0041C584: last.
        cmp eax, edx // 0041C588.
        je finished // 0041C58A: do not read value on empty range.
        mov ecx, dword ptr [esp + 0ch] // 0041C58C: value pointer.
    next_element:
        fld dword ptr [ecx] // 0041C590: reread first source word.
        add eax, 8 // 0041C592: native32 address increment.
        cmp eax, edx // 0041C595: flags retained across both stores.
        fstp dword ptr [eax - 8] // 0041C597: first write before second read.
        fld dword ptr [ecx + 4] // 0041C59A: alias-sensitive second reread.
        fstp dword ptr [eax - 4] // 0041C59D.
        jne next_element // 0041C5A0.
    finished:
        ret // 0041C5A2: EAX is cursor, not a source-level void return.
    }
}
} // namespace bsp
