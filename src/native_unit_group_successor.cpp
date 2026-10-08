#include "bsp/native_unit_group_successor.hpp"

namespace bsp {
static_assert(sizeof(void*) == 4);

__declspec(naked) void* __fastcall native_unit_group_successor_0070d8d0(
    void*, void*, const void*) noexcept {
    __asm {
        push ecx
        mov edx, [esp + 8]
        push esi
        xor esi, esi
        cmp edx, esi
        jz select_or_type18
        mov eax, [ecx + 14h]
        cmp eax, esi
        jz select_or_type18
        cmp edx, eax
        jnz return_identity
    select_or_type18:
        cmp dword ptr [ecx + 4fch], 18h
        mov [esp + 4], esi
        jz load_saved_identity
        push edi
        or edi, -1
        cmp [ecx + 4f8h], esi // Initial count read precedes the column store.
        mov [ecx + 500h], esi
        jle finish_scan
        push ebx
        push ebp
        mov ebp, -38h
        lea ebx, [edi - 33h]
        lea edx, [ecx + 38h]
        sub ebp, ecx
        // Original seven-byte LEA ESP,[ESP+0] alignment instruction.
        _emit 08dh
        _emit 0a4h
        _emit 024h
        _emit 000h
        _emit 000h
        _emit 000h
        _emit 000h
    next_member:
        mov eax, [edx - 20h]
        test eax, eax
        jz advance_member
        cmp byte ptr [eax + 5dh], 0
        jnz advance_member
        cmp eax, [esp + 18h]
        jz advance_member
        cmp eax, [ecx + 14h]
        jz found_leader
        cmp edi, -1
        jz select_candidate
        fld dword ptr [edx]             // Candidate key first.
        fld dword ptr [ebx + ecx + 38h] // Current best key second.
        fcomip st(0), st(1)
        fstp st(0)
        jbe advance_member             // Equal and unordered keep the first.
    select_candidate:
        mov edi, esi
        lea ebx, [edx + ebp]
    advance_member:
        add esi, 1
        add edx, 34h
        cmp esi, [ecx + 4f8h] // 0070D954: reload, not a captured loop bound.
        jl next_member
        jmp choose_result
    found_leader:
        mov [esp + 10h], eax
    choose_result:
        test edi, edi
        pop ebp
        pop ebx
        jl finish_scan
        imul edi, edi, 34h
        mov eax, [edi + ecx + 18h] // Reload the chosen actual entity identity.
        mov [esp + 8], eax         // Overrides an encountered leader if any.
    finish_scan:
        pop edi
    load_saved_identity:
        mov eax, [esp + 4]
    return_identity:
        pop esi
        pop ecx
        ret 4
    }
}
} // namespace bsp
