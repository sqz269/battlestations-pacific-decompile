#include "bsp/native_unit_group_member_vector.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error The complete raw member-vector entry requires MSVC Win32.
#endif

namespace bsp {

__declspec(naked) void* __fastcall copy_native_unit_group_member_vector_0070d980(
    void*, void*, void*, const void*) noexcept {
    __asm {
        push esi
        mov esi, dword ptr [ecx + 04F8h]
        xor eax, eax
        test esi, esi
        push edi
        jle missing
        mov edi, dword ptr [esp + 010h]
        lea edx, [ecx + 018h]
    search:
        cmp dword ptr [edx], edi
        jz found
        add eax, 1
        add edx, 034h
        cmp eax, esi
        jl search
    missing:
        mov eax, dword ptr [esp + 00Ch]
        // Original MOVSS XMM0,[00F87574]. Avoid MSVC's redundant DS prefix
        // for numeric absolute operands; these are native DATA reads.
        _emit 0F3h
        _emit 00Fh
        _emit 010h
        _emit 005h
        _emit 074h
        _emit 075h
        _emit 0F8h
        _emit 000h
        movss dword ptr [eax], xmm0
        // Original MOVSS XMM0,[00F87578].
        _emit 0F3h
        _emit 00Fh
        _emit 010h
        _emit 005h
        _emit 078h
        _emit 075h
        _emit 0F8h
        _emit 000h
        movss dword ptr [eax + 4], xmm0
        // Original MOVSS XMM0,[00F8757C].
        _emit 0F3h
        _emit 00Fh
        _emit 010h
        _emit 005h
        _emit 07Ch
        _emit 075h
        _emit 0F8h
        _emit 000h
        pop edi
        movss dword ptr [eax + 8], xmm0
        pop esi
        ret 8
    found:
        imul eax, eax, 034h
        fld dword ptr [eax + ecx*1 + 01Ch]
        lea ecx, [eax + ecx*1 + 01Ch]
        mov eax, dword ptr [esp + 00Ch]
        pop edi
        pop esi
        fstp dword ptr [eax]
        fld dword ptr [ecx + 4]
        fstp dword ptr [eax + 4]
        fld dword ptr [ecx + 8]
        fstp dword ptr [eax + 8]
        ret 8
    }
}

} // namespace bsp
