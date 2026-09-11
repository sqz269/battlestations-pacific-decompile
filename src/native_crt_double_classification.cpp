#include "bsp/native_crt_double_classification.hpp"

namespace bsp {

// Full original __sptype: 91 bytes.
__declspec(naked) std::int32_t __cdecl native_crt_sptype_00c12f3e(
    std::uint32_t, std::uint32_t, std::uint32_t) {
    __asm {
        push ebp
        mov ebp, esp
        xor edx, edx
        cmp dword ptr [ebp + 0ch], 07ff00000h
        jne native_00c12f56
        cmp dword ptr [ebp + 8], edx
        jne native_00c12f69
        xor eax, eax
        inc eax
        pop ebp
        ret
    native_00c12f56:
        cmp dword ptr [ebp + 0ch], 0fff00000h
        jne native_00c12f69
        cmp dword ptr [ebp + 8], edx
        jne native_00c12f69
        push 2
    native_00c12f66:
        pop eax
        pop ebp
        ret
    native_00c12f69:
        mov ecx, dword ptr [ebp + 0eh]
        mov eax, 07ff8h
        and ecx, eax
        cmp cx, ax
        jne native_00c12f7c
        push 3
        jmp native_00c12f66
    native_00c12f7c:
        cmp cx, 07ff0h
        jne native_00c12f95
        test dword ptr [ebp + 0ch], 07ffffh
        jne native_00c12f91
        cmp dword ptr [ebp + 8], edx
        je native_00c12f95
    native_00c12f91:
        push 4
        jmp native_00c12f66
    native_00c12f95:
        xor eax, eax
        pop ebp
        ret
    }
}

// Full original __fpclass: 148 bytes.
__declspec(naked) std::int32_t __cdecl native_crt_fpclass_00bfa52f(
    std::uint32_t, std::uint32_t, std::uint32_t) {
    __asm {
        push ebp
        mov ebp, esp
        mov eax, dword ptr [ebp + 0eh]
        mov ecx, 07ff0h
        mov edx, eax
        and edx, ecx
        cmp dx, cx
        jne native_00bfa570
        fld qword ptr [ebp + 8]
        push ecx
        push ecx
        fstp qword ptr [esp]
        call native_crt_sptype_00c12f3e
        dec eax
        pop ecx
        pop ecx
        je native_00bfa569
        dec eax
        je native_00bfa564
        dec eax
        je native_00bfa560
        xor eax, eax
        inc eax
        pop ebp
        ret
    native_00bfa560:
        push 2
        jmp native_00bfa566
    native_00bfa564:
        push 4
    native_00bfa566:
        pop eax
        pop ebp
        ret
    native_00bfa569:
        mov eax, 0200h
        pop ebp
        ret
    native_00bfa570:
        and eax, 08000h
        test dx, dx
        mov ecx, eax
        jne native_00bfa599
        test dword ptr [ebp + 0ch], 0fffffh
        jne native_00bfa58b
        cmp dword ptr [ebp + 8], 0
        je native_00bfa599
    native_00bfa58b:
        neg eax
        sbb eax, eax
        and eax, 0ffffff90h
        add eax, 080h
        pop ebp
        ret
    native_00bfa599:
        fldz
        fcomp qword ptr [ebp + 8]
        fnstsw ax
        test ah, 044h
        mov eax, ecx
        jp native_00bfa5b3
        neg eax
        sbb eax, eax
        and eax, 0ffffffe0h
        add eax, 040h
        pop ebp
        ret
    native_00bfa5b3:
        neg eax
        sbb eax, eax
        and eax, 0ffffff08h
        add eax, 0100h
        pop ebp
        ret
    }
}

// Full original __frnd: 17 bytes.
__declspec(naked) void __cdecl native_crt_frnd_st0_00c28548(
    std::uint32_t, std::uint32_t) {
    __asm {
        push ecx
        push ecx
        fld qword ptr [esp + 0ch]
        frndint
        fstp qword ptr [esp]
        fld qword ptr [esp]
        pop ecx
        pop ecx
        ret
    }
}

} // namespace bsp
