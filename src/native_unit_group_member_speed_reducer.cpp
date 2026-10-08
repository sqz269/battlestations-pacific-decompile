#include "bsp/native_unit_group_member_speed_reducer.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error The complete raw member-speed reducer requires MSVC Win32.
#endif

namespace bsp {

__declspec(naked) float __fastcall reduce_native_unit_group_member_speeds_0070d140(
    void*, void*) noexcept {
    __asm {
        sub esp, 8
        mov eax, dword ptr [ecx + 04F8h]
        test eax, eax
        // MOVSS XMM0,[00CFD6F4]. MSVC requires an explicit DS override for
        // this numeric operand; emit the original instruction without it.
        _emit 0F3h
        _emit 00Fh
        _emit 010h
        _emit 005h
        _emit 0F4h
        _emit 0D6h
        _emit 0CFh
        _emit 000h
        movss dword ptr [esp], xmm0
        jle done
        // MOVSS XMM0,[00CF4888], reached only for a positive captured count.
        _emit 0F3h
        _emit 00Fh
        _emit 010h
        _emit 005h
        _emit 088h
        _emit 048h
        _emit 0CFh
        _emit 000h
        add ecx, 048h
        mov edx, eax
    next_record:
        cmp dword ptr [ecx - 030h], 0
        jz null_member
        fld dword ptr [ecx]
        fstp dword ptr [esp + 4]
        fld dword ptr [esp + 4]
        fld dword ptr [esp]
        fcomip st(0), st(1)
        fstp st(0)
        jbe advance
        movss xmm1, dword ptr [esp + 4]
        movss dword ptr [esp], xmm1
        jmp advance
    null_member:
        movss dword ptr [ecx], xmm0
    advance:
        add ecx, 034h
        sub edx, 1
        jnz next_record
    done:
        fld dword ptr [esp]
        add esp, 8
        ret
    }
}

} // namespace bsp
